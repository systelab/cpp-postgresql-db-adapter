#pragma once

#include "DbAdapterInterface/IRecordSet.h"

typedef struct pg_result PGresult;

namespace systelab::db {
	class IField;
	class IRecord;
}

namespace systelab::db::postgresql {

	class RecordSet : public IRecordSet
	{
	public:
		RecordSet(const PGresult* statementResult);
		~RecordSet() override;

		unsigned int getFieldsCount() const override;
		const IField& getField(unsigned int index) const override;
		const IField& getField(const std::string& fieldName) const override;

		unsigned int getRecordsCount() const override;

		const IRecord& getCurrentRecord() const override;
		std::unique_ptr<IRecord> copyCurrentRecord() const override;
		bool isCurrentRecordValid() const override;
		void nextRecord() override;

	private:
		std::vector<std::unique_ptr<IField>> m_fields;
		std::vector<std::unique_ptr<IRecord>> m_records;
		std::vector<std::unique_ptr<IRecord>>::iterator m_iterator;

		void createFields(const PGresult* statementResult);
	};
}