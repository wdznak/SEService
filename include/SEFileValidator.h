#pragma once
#include <algorithm>
#include <memory>
#include <vector>

#include "SENamespace.h"
#include "BinanceInfo.h"
#include "ConnectionVisitor.h"

#include "rapidjson/document.h"

namespace seservice {

	class SEFileValidator : public ConnectionVisitor
	{
	public:
		using ExchangeList = std::vector<std::string>;
		using Data_t = rapidjson::GenericArray<true, rapidjson::Value::ValueType>;
		typedef rapidjson::Value value_type;

	private:
		std::string fileName_;
		rapidjson::Document document_;
		ExchangeList exchangeList_;

	public:
		SEFileValidator(const std::string& fileName);

		bool validate();
		const ExchangeList& exchangeList() const;

		/***** Temp func begin *****/
		void visitBinance(BinanceInfo& info);
		/***** Temp func end *****/

	private:
		void createList();
	};

} // namespace seservice