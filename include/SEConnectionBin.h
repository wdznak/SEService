#pragma once

#include <condition_variable>
#include <mutex>
#include <memory>
#include <optional>
#include <unordered_map>
#include <utility>
#include <vector>

#include <boost/signals2.hpp>
#include <tbb/concurrent_queue.h>
#include <uWS/uWS.h>

#include "SENamespace.h"

namespace seservice {
	
	using namespace uWS;
	using namespace uS;

	class SEConnectionBin
	{
	public:
		struct BinData {
			int id;
			std::string address;
			std::string description;
			std::string depthAddress;
			std::string symbol;
		};

		using errorCb_t = boost::signals2::signal<void(int, std::string, ErrorCode)>;
		using messageCb_t = std::function<void(int, std::string)>;

	private:
		struct WebSocketData {
			int uniqueId = -1;
			messageCb_t messageCb;
			BinData data;
			SEConnectionBin* connection;
			int reconnect = 4;
			uWS::WebSocket<CLIENT>* ws;
			void addConnection() {
				connection->addConnection(this);
			};
		};

		using depthPair_t = std::pair<std::string, std::string>;
		using activeWebSockets_t = std::unordered_map<int, std::unique_ptr<WebSocketData>>;

		activeWebSockets_t activeWebSockets_;
		std::condition_variable cv_;
		const errorCb_t& errorSig_;
		std::unique_ptr<Hub> hub_;
		std::mutex m_;
		std::unique_ptr<std::thread> t_;
		bool waitForConnection_ = true;

	public:
		SEConnectionBin(const errorCb_t& errorSig);
		~SEConnectionBin();

		void close(int uniqueId);
		// After call to "closeAll" object has to be destroyed!
		void closeAll();
		void connect(int uniqueId, messageCb_t messageCb, BinData* data);

	private:
		void addConnection(WebSocketData*);
		std::optional<std::string> getDepthSnapshot(int uniqueId, const std::string& depthAddress);
		void spawn();

		friend void WebSocketData::addConnection();
	};

}

