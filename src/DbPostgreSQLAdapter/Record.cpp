#include "stdafx.h"
#include "Record.h"

#include "DbAdapterInterface/IRecordSet.h"
#include "DbAdapterInterface/Types.h"
#include "Field.h"
#include "FieldValue.h"
#include "PostgresUtils.h"

namespace systelab::db::postgresql {

	Record::Record(IRecordSet& recordSet, const PGresult* statementResult, const int rowIndex)
	{
		const unsigned int fieldsCount = recordSet.getFieldsCount();
		for (unsigned int i = 0; i < fieldsCount; i++)
		{
			std::unique_ptr<IFieldValue> fieldValue;
			const IField& field = recordSet.getField(i);
			const unsigned int fieldIndex = field.getIndex();
			if (PQgetisnull(statementResult, rowIndex, fieldIndex) == 1)
			{
				fieldValue = std::make_unique<FieldValue>(field);
			}
			else
			{
				const FieldTypes fieldType = field.getType();
				const std::string value = PQgetvalue(statementResult, rowIndex, fieldIndex);
				switch(fieldType)
				{
					case BOOLEAN:
						fieldValue = std::make_unique<FieldValue>(field, (value == "t"));
						break;
					case INT:
						fieldValue = std::make_unique<FieldValue>(field, std::stoi(value));
						break;
					case DOUBLE:
						fieldValue = std::make_unique<FieldValue>(field, std::stod(value));
						break;
					case STRING:
						fieldValue = std::make_unique<FieldValue>(field, std::string(value));
						break;

					case DATETIME:
						fieldValue = std::make_unique<FieldValue>(field, utils::stringISOToDateTime(value));
						break;
					case BINARY:
					default:
						throw std::runtime_error( "Unknown field type." );
				}
			}

			m_fieldValues.push_back(std::move(fieldValue));
		}
	}

	Record::Record(std::vector<std::unique_ptr<IFieldValue>>& fieldValues)
	{
		const unsigned int nFieldValues = static_cast<unsigned int>(fieldValues.size());
		for (unsigned int i = 0; i < nFieldValues; i++)
		{
			m_fieldValues.push_back(std::move(fieldValues.at(i)));
		}
	}

	unsigned int Record::getFieldValuesCount() const
	{
		return static_cast<unsigned int>(m_fieldValues.size());
	}

	IFieldValue& Record::getFieldValue(unsigned int index) const
	{
		return *m_fieldValues.at(index);
	}

	IFieldValue& Record::getFieldValue(const std::string& fieldName) const
	{
		const auto fieldValueIterator = std::ranges::find_if(m_fieldValues,
			[&fieldName](const std::unique_ptr<IFieldValue>& fieldValue)
			{
				return fieldValue->getField().getName() == fieldName;
			});

		if (fieldValueIterator != m_fieldValues.cend())
		{
			return **fieldValueIterator;
		}


		throw std::runtime_error( "The requested field value doesn't exist" );
	}

	bool Record::hasFieldValue(const std::string& fieldName) const
	{
		const auto fieldValueIterator = std::ranges::find_if(m_fieldValues,
			[&fieldName](const std::unique_ptr<IFieldValue>& fieldValue)
			{
				return fieldValue->getField().getName() == fieldName;
			});

		return fieldValueIterator != m_fieldValues.cend();
	}
}