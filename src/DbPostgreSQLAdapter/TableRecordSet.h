#pragma once

#include "DbAdapterInterface/ITableRecordSet.h"

typedef struct pg_result PGresult;

namespace systelab { namespace db {
	class IField;
	class ITable;
	class ITableRecord;
}}

namespace systelab { namespace db { namespace postgresql {

	class TableRecordSet : public ITableRecordSet
	{
	public:
		TableRecordSet(ITable& table, const PGresult* statementResult);
		~TableRecordSet() override = default;

		ITable& getTable() const override;

		unsigned int getFieldsCount() const override;
		const IField& getField(unsigned int index) const override;
		const IField& getField(const std::string& fieldName) const override;

		unsigned int getRecordsCount() const override;

		const ITableRecord& getCurrentRecord() const override;
		std::unique_ptr<ITableRecord> copyCurrentRecord() const override;
		bool isCurrentRecordValid() const override;
		void nextRecord() override;

	private:
		ITable& m_table;
		std::vector<std::unique_ptr<ITableRecord>> m_records;
		std::vector< std::unique_ptr<ITableRecord>>::iterator m_iterator;
	};
}}}