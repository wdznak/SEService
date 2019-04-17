#pragma once

namespace seservice {
	
	class BinanceInfo;

	class ConnectionVisitor
	{
	public:
		virtual ~ConnectionVisitor() = default;
		virtual void visitBinance(BinanceInfo&) = 0;
	};

} // namespace seservice

