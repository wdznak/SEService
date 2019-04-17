#pragma once
#include <ostream>
#include <tuple>
#include <utility>

#include "ConnectionVisitor.h"
#include "SEConnectionI.h"
#include "SENamespace.h"

namespace seservice {

	class ConnectionInfo {
	public:
		int id;

		virtual ~ConnectionInfo() = default;

		virtual void accept(ConnectionVisitor&) = 0;
		virtual void connect(short, int, const SEConnectionI::messageCb_t&, const SEConnectionI::errorCb_t&) = 0;
		virtual void close(int) = 0;
		virtual void closeAll() = 0;
		virtual std::vector<std::tuple<int, std::string, std::string>> getConnections() const = 0;
	};

} // namespace seservice
