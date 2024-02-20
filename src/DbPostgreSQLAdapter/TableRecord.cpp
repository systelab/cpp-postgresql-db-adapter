#include "stdafx.h"
#include "TableRecord.h"

#include "Field.h"
#include "FieldValue.h"

#include "DbAdapterInterface/ITableRecordSet.h"
#include "PostgresUtils.h"

namespace systelab::db::postgresql {

	TableRecord::TableRecord(ITableRecordSet& recordSet, const PGresult* statementResult, const int rowIndex)
		: m_table(recordSet.getTable())
	{
		const unsigned int fieldsCount = recordSet.getFieldsCount();
		for (unsigned int i = 0; i < fieldsCount; i++)
		{
			std::unique_ptr<IFieldValue> fieldValue;
			const IField& field = recordSet.getField(i);
			const unsigned int fieldIndex = field.getIndex();
			if (PQgetisnull(statementResult, rowIndex, fieldIndex) == 1)
			{
				fieldValue.reset(new FieldValue(field));
			}
			else
			{
				FieldTypes fieldType = field.getType();
				const std::string value = PQgetvalue(statementResult, rowIndex, fieldIndex);
				switch(fieldType)
				{
					case BOOLEAN:
					{
						bool boolValue = (value == "t");
						fieldValue.reset(new FieldValue(field, boolValue));
					}
					break;

					case INT:
					{
						int intValue = std::stoi(value);
						fieldValue.reset(new FieldValue(field, intValue));
					}
					break;

					case DOUBLE:
					{
						double doubleValue = std::stod(value);
						fieldValue.reset(new FieldValue(field, doubleValue));
					}
					break;

					case STRING:
					{
						std::string stringValue(value);
						fieldValue.reset(new FieldValue(field, stringValue));
					}
					break;

					case DATETIME:
					{
						std::string stringValue(value);
						fieldValue.reset(new FieldValue(field, utils::stringISOToDateTime(stringValue)));
					}
					break;

					case BINARY:
					default:
						throw std::runtime_error( "Unknown field type." );
				}
			}

			m_fieldValues.push_back(std::move(fieldValue));
		}
	}

	TableRecord::~TableRecord()
	{}

	TableRecord::TableRecord(ITable& table, std::vector< std::unique_ptr<IFieldValue> >& fieldValues)
		: m_table(table)
	{
		const unsigned int fieldValuesCount = static_cast<unsigned int>(fieldValues.size());
		for (unsigned int i = 0; i < fieldValuesCount; i++)
		{
			m_fieldValues.push_back(std::move(fieldValues.at(i)));
		}
	}

	ITable& TableRecord::getTable() const
	{
		return m_table;
	}

	unsigned int TableRecord::getFieldValuesCount() const
	{
		return static_cast<unsigned int>(m_fieldValues.size());
	}

	IFieldValue& TableRecord::getFieldValue(unsigned int index) const
	{
		return *m_fieldValues.at(index);
	}

	IFieldValue& TableRecord::getFieldValue(const std::string& fieldName) const
	{
		const auto fieldValueIterator = std::find_if(m_fieldValues.cbegin(), m_fieldValues.cend(),
			[&fieldName](const std::unique_ptr<IFieldValue>& fieldValue)
			{
				return fieldValue->getField().getName() == fieldName;
			});

		if (fieldValueIterator != m_fieldValues.cend())
		{
			return *(fieldValueIterator->get());
		}
		
		throw std::runtime_error( "The requested field value doesn't exist" );
	}

	bool TableRecord::hasFieldValue(const std::string& fieldName) const
	{
		const auto fieldValueIterator = std::find_if(m_fieldValues.cbegin(), m_fieldValues.cend(),
			[&fieldName](const std::unique_ptr<IFieldValue>& fieldValue)
			{
				return fieldValue->getField().getName() == fieldName;
			});

		return fieldValueIterator != m_fieldValues.cend();
	}

	std::vector<IFieldValue*> TableRecord::getValuesList() const
	{
		std::vector<IFieldValue*> values;
		const unsigned int recordFieldValuesCount = getFieldValuesCount();

		std::for_each(m_fieldValues.cbegin(), m_fieldValues.cend(),
			[this, &values](const std::unique_ptr<IFieldValue>& fieldValue)
			{
				if (!fieldValue->getField().isPrimaryKey())
				{
					values.push_back(fieldValue.get());
				}
			});

		return values;
	}
}