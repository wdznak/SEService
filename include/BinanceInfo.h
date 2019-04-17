#pragma once
#include <algorithm>
#include <memory>
#include <ostream>
#include <string>
#include <tuple>
#include <utility>

#include "ConnectionInfo.h"
#include "SEConnectionBin.h"

#include <chrono>
#include <thread>

namespace seservice {

class BinanceInfo : public ConnectionInfo
{
public:
	using data_t = SEConnectionBin::BinData;
	using connectionList_t = std::vector<std::tuple<int, std::string, std::string>>;

private:
	using dataList_t = std::unique_ptr<data_t>;

	std::unique_ptr<SEConnectionBin> connection_;
	std::vector<dataList_t> dataList_;
	const std::string exchangeName = "Binance";

public:
	BinanceInfo() {}
	~BinanceInfo() {}

	void accept(ConnectionVisitor& visitor) override {
		visitor.visitBinance(*this);
	}

	void add(std::unique_ptr<data_t>& data) {
		dataList_.push_back(std::move(data));
	}

	void connect(short connectionId, int uniqueId, const SEConnectionI::messageCb_t& messageCb, const SEConnectionI::errorCb_t& errorSig) override {
		if (!connection_) {
			connection_ =  std::make_unique<SEConnectionBin>(errorSig);
		}

		auto connection = std::find_if(dataList_.begin(), dataList_.end(), [&connectionId](const dataList_t& data) {
			return data->id == connectionId;
		});
		
		if (connection != dataList_.end()) {
			connection_->connect(uniqueId, messageCb, connection->get());
		}
	}

	void close(int uniqueId) override {
		if (connection_) connection_->close(uniqueId);
	}

	void closeAll() override {
		if (connection_) connection_->closeAll();
	}

	std::string getExchangeName() const {
		return exchangeName;
	}

	connectionList_t getConnections() const override {
		connectionList_t connectionsList;

		std::for_each(dataList_.begin(), dataList_.end(), [&](const dataList_t& data) {
			connectionsList.push_back({ data->id, data->symbol, data->description });
		});

		return connectionsList;
	}
};

} // namespace seservice