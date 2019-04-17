#include "stdafx.h"

#include "SEConnection.h"

#include <algorithm>
#include <limits>
#include <memory>
#include <unordered_map>

#include "SEConnectionI.h"
#include "ConnectionInfoGen.h"
#include "SEFileValidator.h"

namespace seservice {
	using namespace rapidjson;

	class SEConnection::Impl
	{
	private:
		const char* fileName_ = "test.json";
		boost::signals2::signal<errorCb_t> errorSignal_;
		std::unordered_map<std::string, std::unique_ptr<ConnectionInfo>> connectionList_;

	public:
		Impl(const std::function<errorCb_t>& userErrorCb) {
			errorSignal_.connect(userErrorCb);

			readConnections();
		}
		
		// Map structure
		// { Exchange Name } -> vector { Connection ID, Connection Description }
		connList_t availableConnections() const {
			connList_t connections;

			for (auto& connection : connectionList_) {
				connections.insert({connection.first, connection.second->getConnections()});
			}

			return connections;
		}

		int connect(const std::string& exchangeName, int connectionId, const messageCb_t& messageCb) 
		{
			int uniqueId = -1;
			if (!connectionList_.count(exchangeName)) {
				errorSignal_(connectionId, "Exchange does not exists.", BADREQUEST);
				return uniqueId;
			}

			uniqueId = getId();
			if (-1 == uniqueId) return uniqueId;

			auto& connection = connectionList_.at(exchangeName);

			connection->connect(connectionId, uniqueId, messageCb, errorSignal_);
			
			return uniqueId;
		}

		void close(int uniqueId, const std::string& exchangeName) {
			if (!connectionList_.count(exchangeName)) {
				errorSignal_(uniqueId, "Connection does not exists.", BADREQUEST);
				return;
			}

			auto& connection = connectionList_.at(exchangeName);

			connection->close(uniqueId);
		}

		void closeAll() {
			std::for_each(connectionList_.begin(), connectionList_.end(), [](auto& connection) {
				connection.second->closeAll();
			});
		}

	private:
		// Fancy id generator
		int getId() {
			static int id = 0;
			if (id < std::numeric_limits<int>::max()) {
				return id++;
			}
			return -1;
		}

		void readConnections() {
			SEFileValidator validator(fileName_);
			if (validator.validate()) {
				std::unique_ptr<ConnectionInfo> connInfo;
				for(auto& exchange : validator.exchangeList()) {
					if (connInfo = ConnectionInfoGen::generate(exchange)) {
						connInfo->accept(validator);
						connectionList_.insert({ exchange, std::move(connInfo) });
					}
				}
			}
			else {
				errorSignal_(-1, "Can't validate file\n", CANTREADFILE);
			}
		}

	};

	SEConnection::SEConnection(const std::function<errorCb_t>& userErrorCb)
		: impl_(std::make_unique<Impl>(userErrorCb))
	{
	}
 
	// Destructor must be declared here because of unique_ptr<Impl>.
	// It can't delete incomplete type.
	// In the future unique_ptr may be changed to "spimpl".
	SEConnection::~SEConnection() = default;

	SEConnection::connList_t SEConnection::availableConnections() const
	{
		return impl_->availableConnections();
	}

	void SEConnection::close(int uniqueId, const std::string& exchangeName)
	{
		impl_->close(uniqueId, exchangeName);
	}

	void SEConnection::closeAll()
	{
		impl_->closeAll();
	}
	
	int SEConnection::connect(const std::string& exchangeName, int connectionId, const messageCb_t& messageCb)
	{
		return impl_->connect(exchangeName, connectionId, messageCb);
	}
	;
} // namespace seservice