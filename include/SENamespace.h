#pragma once

namespace seservice {

	enum ErrorCode {
		OK = 0,
		BADFILE,
		BADREQUEST,
		CANTREADFILE,
		CONNECTIONDUPLICATED,
		USERDISCONNECTED,
		INVALIDDATA
	};

} // namespace seservice
