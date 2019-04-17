#pragma once

#include <functional>
#include <map>
#include <string>
#include <tuple>
#include <vector>

#include <boost/signals2.hpp>

#include "SENamespace.h"

namespace seservice {
	// SEConnection is not thread safe.
	class SEConnection
	{
	public:
		using connList_t = std::map<std::string, std::vector<std::tuple<int, std::string, std::string>>>;
		using errorCb_t = void(int, std::string, ErrorCode);
		using messageCb_t = std::function<void(int, std::string)>;

	private:
		class Impl;
		std::unique_ptr<Impl> impl_;

	public:
		SEConnection(const std::function<errorCb_t>& userErrorCb);
		~SEConnection();

		connList_t availableConnections() const;
		void close(int uniqueId, const std::string& exchangeName);
		void closeAll();
		int connect(const std::string& exchangeName, int connectionId, const messageCb_t& messageCb);
	};
} // namespace seservice

