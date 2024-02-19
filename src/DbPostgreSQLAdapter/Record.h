#pragma once

#include "DbAdapterInterface/IRecord.h"

namespace systelab::db {
	class IFieldValue;
	class IRecordSet;
}

namespace systelab::db::postgresql {

	class Record : public IRecord
	{
	public:
		Record(IRecordSet& recordSet, const PGresult* statementResult, const int rowIndex);
		Record(std::vector<std::unique_ptr<IFieldValue>>&);
		~Record() override = default;

		unsigned int getFieldValuesCount() const override;
		IFieldValue& getFieldValue(unsigned int index) const override;
		IFieldValue& getFieldValue(const std::string& fieldName) const override;

		bool hasFieldValue(const std::string& fieldName) const override;

	private:
		std::vector<std::unique_ptr<IFieldValue>> m_fieldValues;
	};
}