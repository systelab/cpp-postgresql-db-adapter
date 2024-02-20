#include "stdafx.h"
#include "RecordSet.h"

#include "DefaultOID.h"
#include "Field.h"
#include "Record.h"

#include "DbAdapterInterface/IFieldValue.h"

namespace systelab::db::postgresql {

	namespace {
		FieldTypes translateFromOIDtoFieldType(const PostgresqlOID oid)
		{
			switch (oid)
			{
				case(PostgresqlOID::boolOID):
					return FieldTypes::BOOLEAN;
					break;
				case(PostgresqlOID::bytearrayOID):
					return FieldTypes::BINARY;
					break;
				case(PostgresqlOID::smallIntIOD):
				case(PostgresqlOID::intOID):
					return FieldTypes::INT;
					break;
				case(PostgresqlOID::pidOID):
				case(PostgresqlOID::nameOID):
				case(PostgresqlOID::charOID):
				case(PostgresqlOID::textOID):
				case(PostgresqlOID::varcharOID):
					return FieldTypes::STRING;
					break;
				case(PostgresqlOID::floatOID):
				case(PostgresqlOID::doubleOID):
					return FieldTypes::DOUBLE;
					break;
				case(PostgresqlOID::datetimeOID):
					return FieldTypes::DATETIME;
					break;
			}

			throw ("OID type not defined");
		}
	}

	RecordSet::RecordSet(const PGresult* statementResult)
	{
		createFields(statementResult);

		const unsigned int rowsCount = static_cast<unsigned int>(PQntuples(statementResult));
		for (unsigned int i = 0; i < rowsCount; i++)
		{
			m_records.push_back(std::make_unique<Record>(*this, statementResult, i));
		}

		m_iterator = m_records.begin();
	}

	RecordSet::~RecordSet()
	{

	}

	unsigned int RecordSet::getFieldsCount() const
	{
		return (unsigned int) m_fields.size();
	}

	const IField& RecordSet::getField(unsigned int index) const
	{
		return *m_fields.at(index);
	}

	const IField& RecordSet::getField(const std::string& fieldName) const
	{
		unsigned int nFields = (unsigned int) m_fields.size();
		for (unsigned int i = 0; i < nFields; i++)
		{
			if (m_fields.at(i)->getName() == fieldName)
			{
				return *m_fields.at(i);
			}
		}

		throw std::runtime_error( "The requested field doesn't exist" );
	}

	unsigned int RecordSet::getRecordsCount() const
	{
		return (unsigned int) m_records.size();
	}

	const IRecord& RecordSet::getCurrentRecord() const
	{
		return **m_iterator;
	}

	std::unique_ptr<IRecord> RecordSet::copyCurrentRecord() const
	{
		const IRecord& currentRecord = getCurrentRecord();

		std::vector< std::unique_ptr<IFieldValue> > copiedFieldValues;
		unsigned int nFieldValues = currentRecord.getFieldValuesCount();
		for (unsigned int i = 0; i < nFieldValues; i++)
		{
			IFieldValue& fieldValue = currentRecord.getFieldValue(i);
			copiedFieldValues.push_back( fieldValue.clone() );
		}

		return std::unique_ptr<IRecord>( new Record(copiedFieldValues) );
	}

	bool RecordSet::isCurrentRecordValid() const
	{
		return (m_iterator != m_records.end());
	}

	void RecordSet::nextRecord()
	{
		m_iterator++;
	}

	void RecordSet::createFields(const PGresult* statementResult)
	{
		int fieldsCount = PQnfields(statementResult);
		for (int i = 0; i < fieldsCount; i++)
		{
			std::string fieldName(PQfname(statementResult, i));
			int postgresqlFieldType = PQftype(statementResult, i);

			FieldTypes fieldType = translateFromOIDtoFieldType(static_cast<PostgresqlOID>(postgresqlFieldType));
			std::unique_ptr<IField> field(new Field(i, fieldName, fieldType, "", false));
			m_fields.push_back(std::move(field));
		}
	}
}