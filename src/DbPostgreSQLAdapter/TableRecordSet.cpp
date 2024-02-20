#include "stdafx.h"
#include "TableRecordSet.h"

#include "Field.h"
#include "TableRecord.h"

#include "DbAdapterInterface/ITable.h"

namespace systelab::db::postgresql {

	TableRecordSet::TableRecordSet(ITable& table, const PGresult* statementResult)
		: m_table(table)
	{
		const unsigned int rowsCount = static_cast<unsigned int>(PQntuples(statementResult));
		for(unsigned int i = 0; i < rowsCount; i++)
		{
			m_records.push_back(std::make_unique<TableRecord>(*this, statementResult, i));
		}

		m_iterator = m_records.begin();
	}

	ITable& TableRecordSet::getTable() const
	{
		return m_table;
	}

	unsigned int TableRecordSet::getFieldsCount() const
	{
		return m_table.getFieldsCount();
	}

	const IField& TableRecordSet::getField(unsigned int index) const
	{
		return m_table.getField(index);
	}

	const IField& TableRecordSet::getField(const std::string& fieldName) const
	{
		return m_table.getField(fieldName);
	}

	unsigned int TableRecordSet::getRecordsCount() const
	{
		return static_cast<unsigned int>(m_records.size());
	}

	const ITableRecord& TableRecordSet::getCurrentRecord() const
	{
		return *(m_iterator->get());
	}

	std::unique_ptr<ITableRecord> TableRecordSet::copyCurrentRecord() const
	{
		const ITableRecord& currentRecord = getCurrentRecord();

		std::vector<std::unique_ptr<IFieldValue>> copiedFieldValues;
		const unsigned int nFieldValues = currentRecord.getFieldValuesCount();
		for (unsigned int i = 0; i < nFieldValues; i++)
		{
			copiedFieldValues.push_back(currentRecord.getFieldValue(i).clone());
		}

		return std::make_unique<TableRecord>(m_table, copiedFieldValues);
	}

	bool TableRecordSet::isCurrentRecordValid() const
	{
		return (m_iterator != m_records.end());
	}

	void TableRecordSet::nextRecord()
	{
		m_iterator++;
	}
}