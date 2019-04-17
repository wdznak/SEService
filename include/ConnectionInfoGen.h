#pragma once
#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <string>

#include "ConnectionInfo.h"
#include "BinanceInfo.h"

namespace seservice {

	class ConnectionInfoGen
	{
	public:
		static std::unique_ptr<ConnectionInfo> generate(std::string name) {
			if (!isLoaded) {
				createList();
				isLoaded = true;
			}
			std::transform(name.begin(), name.end(), name.begin(), ::tolower);

			if (connList_.count(name)) {
				return std::unique_ptr<ConnectionInfo>(connList_[name]());
			}

			return nullptr;
		}

	private:
		static bool isLoaded;
		static std::map<std::string, std::function<ConnectionInfo*()>> connList_;
		static void createList() {
			connList_.emplace("binance", []()->ConnectionInfo* { return new BinanceInfo{}; });
			connList_.emplace("bitmex", []()->ConnectionInfo* { return new BinanceInfo{}; });
		}
	};

	bool ConnectionInfoGen::isLoaded = false;
	std::map<std::string, std::function<ConnectionInfo*()>> ConnectionInfoGen::connList_{};

} // namespace seservice

