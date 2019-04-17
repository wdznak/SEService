#include "stdafx.h"
#include "SEFileValidator.h"

#include <fstream>
#include <memory>

#include <rapidjson/istreamwrapper.h>

namespace seservice {

	SEFileValidator::SEFileValidator(const std::string& fileName)
		: fileName_(fileName) {}

	bool SEFileValidator::validate() {
		bool isValid = false;
		std::ifstream stream(fileName_);

		rapidjson::IStreamWrapper wrapper(stream);

		document_.ParseStream(wrapper);

		if (document_.IsObject()) {
			createList();
			isValid = true;
		};

		return isValid;
	}

	const SEFileValidator::ExchangeList& SEFileValidator::exchangeList() const
	{
		return exchangeList_;
	}

	void SEFileValidator::visitBinance(BinanceInfo& info) {
		const rapidjson::Value& obj = document_["Exchanges"];
		const rapidjson::Value& arr = obj["Binance"];

		std::for_each(arr.Begin(), arr.End(), [&](const rapidjson::Value& el) {
			std::unique_ptr<BinanceInfo::data_t> data = std::make_unique<BinanceInfo::data_t>();
			data->id           = el["id"].GetInt();
			data->address      = el["address"].GetString();
			data->description  = el["description"].GetString();
			data->depthAddress = el["depthAddress"].GetString();
			data->symbol	   = el["symbol"].GetString();
			info.add(data);
		});

	}

	void SEFileValidator::createList() {
		for(auto& value: document_["Exchanges"].GetObject()) {
			exchangeList_.emplace_back(value.name.GetString());
		}
	}

} // namespace seservice
