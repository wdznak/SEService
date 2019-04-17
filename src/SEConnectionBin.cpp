#include "stdafx.h"
#include "SEConnectionBin.h"

#include <future>
#include <optional>

#include <boost/algorithm/string.hpp>
#include <cpr/cpr.h>

namespace seservice {
	// TODO: Implement "waitForConnection" in "uv_idle/uv_timer" style
	// but using conditional_variable to cleanup current implementation.
	SEConnectionBin::SEConnectionBin(const errorCb_t& errorSig)
		: errorSig_(errorSig), hub_(std::make_unique<Hub>())
	{		
		hub_->onError([&](void* user) {
			if (activeWebSockets_.size() < 2) {
				std::unique_lock<std::mutex> lock(m_);
				cv_.wait(lock, [&] { return !waitForConnection_; });
				waitForConnection_ = true;
			}

			auto webSocketData = static_cast<WebSocketData*>(user);
		
			errorSig_(webSocketData->uniqueId, "Connection error.", BADREQUEST);
			activeWebSockets_.erase(webSocketData->uniqueId);
		});
		
		hub_->onConnection([&](WebSocket<CLIENT>* ws, HttpRequest req) {
			{
				std::lock_guard<std::mutex> lock(m_);
				waitForConnection_ = true;
			}
			auto webSocketData = static_cast<WebSocketData*>(ws->getUserData());
			webSocketData->ws = ws;
			const std::string& depthAddress = webSocketData->data.depthAddress;

			if (depthAddress != "") {
				if (auto depth = getDepthSnapshot(webSocketData->uniqueId, depthAddress)) {
					webSocketData->messageCb(webSocketData->uniqueId, depth.value());
				}
			}
		});

		hub_->onMessage([&](WebSocket<CLIENT>* ws, char* msg, size_t length, OpCode opCode) {
			auto webSocketData = static_cast<WebSocketData*>(ws->getUserData());
			webSocketData->messageCb(webSocketData->uniqueId, std::string(msg, length));
		});

		hub_->onDisconnection([&](WebSocket<CLIENT>* ws, int code, char* message, size_t length) {
			auto webSocketData = static_cast<WebSocketData*>(ws->getUserData());

			if (code == 1001) {
				// If size is less than 2 it means that this websocket is the last one.
				// Condition variable keeps thread alive until new connection is open
				// or user calls "closeAll()".
				activeWebSockets_.erase(webSocketData->uniqueId);
				if (activeWebSockets_.size() == 0) {
					std::unique_lock<std::mutex> lock(m_);
					cv_.wait(lock, [&] { return !waitForConnection_; });
					waitForConnection_ = true;
				}
			}
			else if (code == 1002) {
				// User called "closeAll()"
				activeWebSockets_.erase(webSocketData->uniqueId);
			}
			else {
				if (webSocketData->reconnect > 0) {
					--webSocketData->reconnect;
					hub_->connect(webSocketData->data.address, (void*)webSocketData);
				}
				else {
					errorSig_(webSocketData->uniqueId, std::string(message, length), BADREQUEST);
					activeWebSockets_.erase(webSocketData->uniqueId);
				}
			}
			
		});
	}

	SEConnectionBin::~SEConnectionBin()
	{
		if (t_ && t_->joinable()) {
			t_->join();
		}
	}

	void SEConnectionBin::close(int uniqueId)
	{
		std::unique_lock<std::mutex> lock(m_);
		// If "waitForConnection_" is false it means that all connections are closed.
		// In that case we do nothing.
		if (waitForConnection_) {
			std::pair<activeWebSockets_t*, int>* data = new std::pair<activeWebSockets_t*, int>{ &activeWebSockets_, uniqueId };
			Async* a = new Async(hub_->getLoop());

			a->setData(static_cast<void*>(data));
			a->start([](Async* request) {
				auto data = static_cast<std::pair<activeWebSockets_t*, int>*>(request->getData());
				int id;
				activeWebSockets_t* activeWebSockets;
				std::tie(activeWebSockets, id) = *data;
				if (activeWebSockets->count(id)) {
					uWS::WebSocket<CLIENT>* webSocket = activeWebSockets->at(id)->ws;
					webSocket->close(1001, "Socket closed by user.");
				}
				delete data;
				request->close();
			});

			a->send();
		}
	}

	void SEConnectionBin::closeAll()
	{
		std::unique_lock<std::mutex> lock(m_);
		waitForConnection_ = false;

		Async* a = new Async(hub_->getLoop());
		std::promise<bool> isStoppedP;
		std::future<bool> isStoppedF = isStoppedP.get_future();
		auto data = new std::pair<Hub*, std::promise<bool>>{ hub_.get(), std::move(isStoppedP) };

		a->setData(data);
		a->start([](Async* request) {
			auto data = static_cast<std::pair<Hub*, std::promise<bool>>*>(request->getData());
			std::string str{ "Sockets closed by user." };
			data->first->getDefaultGroup<uWS::CLIENT>().close(1002, str.data());
			request->close();

			data->second.set_value(true);
		});

		a->send();

		lock.unlock();
		cv_.notify_one();

		isStoppedF.wait();
		delete data;
	}

	void SEConnectionBin::connect(int uniqueId, messageCb_t messageCb, BinData* data)
	{
		if (!data) {
			errorSig_(-1, "Invalid data for the connection to Binance!", INVALIDDATA);
			return;
		}

		std::unique_lock<std::mutex> lock(m_);
		waitForConnection_ = false;

		// "webSocketData" is deleted by removing entry from "activeWebSockets" in "onError" or "onDisconnect"
		WebSocketData* webSocketData = new WebSocketData{ uniqueId, messageCb, *data, this };
		Async* a = new Async(hub_->getLoop());

		a->setData(static_cast<void*>(webSocketData));
		a->start([](Async* request) {
			auto webSocketData = static_cast<WebSocketData*>(request->getData());
			webSocketData->addConnection();

			request->close();
		});

		a->send();

		lock.unlock();
		cv_.notify_one();

		if (!t_) {
			t_ = std::make_unique<std::thread>(&SEConnectionBin::spawn, this);
		}
	}

	std::optional<std::string> SEConnectionBin::getDepthSnapshot(int uniqueId, const std::string& depthAddress)
	{
		auto r = cpr::Get(cpr::Url{ depthAddress }, cpr::VerifySsl{ false });

		if (r.status_code == 200) return r.text;

		errorSig_(uniqueId, "Depth snapshot error:" + r.text, BADREQUEST);
		return {};
	}

	void SEConnectionBin::addConnection(WebSocketData* webSocketData)
	{
		auto pair = activeWebSockets_.insert({ webSocketData->uniqueId, std::move(std::unique_ptr<WebSocketData>(webSocketData)) });

		if (pair.second) {
			hub_->connect(webSocketData->data.address, (void*)pair.first->second.get());
		}
	}

	void SEConnectionBin::spawn()
	{
		hub_->run();
	}
}
