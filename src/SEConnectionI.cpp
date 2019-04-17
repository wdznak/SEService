#include "stdafx.h"

#include "SEConnectionI.h"
#include "SEConnectionBin.h"

namespace seservice {

	SEConnectionI::SEConnectionI(const errorCb_t& errorSig)
		: hub_(std::make_unique<Hub>()),errorSig_(errorSig)
	{
	}
} // namespace seservice
