#pragma once

#include <map>

#include <boost/signals2.hpp>
#include <tbb/concurrent_queue.h>
#include <uWS/uWS.h>

#include "SENamespace.h"

namespace seservice {

	using namespace uWS;
	using namespace uS;
	
	class SEConnectionI
	{
	public:
		struct Data{
			virtual ~Data() = default;
		};

		using errorCb_t = boost::signals2::signal<void(int, std::string, ErrorCode)>;
		using messageCb_t = std::function<void(int, std::string)>;

		SEConnectionI(const SEConnectionI&) = delete;
		SEConnectionI& operator=(const SEConnectionI&) = delete;
		SEConnectionI(SEConnectionI&&) = delete;
		SEConnectionI& operator=(SEConnectionI&&) = delete;

		virtual ~SEConnectionI() = default;

		virtual void connect(int uniqueId, messageCb_t messageCb, Data* data) = 0;
		virtual void disconnect(int uniqueId) = 0;

	protected:
		SEConnectionI(const errorCb_t& errorSig);
		std::unique_ptr<Hub> hub_;
		const errorCb_t& errorSig_;
	};

}