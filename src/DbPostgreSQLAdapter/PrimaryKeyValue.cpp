#include "stdafx.h"
#include "PrimaryKeyValue.h"

#include "Field.h"
#include "FieldValue.h"
#include "PrimaryKey.h"

namespace systelab::db::postgresql {

	PrimaryKeyValue::PrimaryKeyValue(const IPrimaryKey& primaryKey)
		:m_primaryKey(primaryKey)
	{
		const unsigned int nPrimaryKeyFields = m_primaryKey.getFieldsCount();
		for (unsigned int i = 0; i < nPrimaryKeyFields; i++)
		{
			std::unique_ptr<IFieldValue> fieldValue;

			const IField& field = primaryKey.getField(i);
			if (field.hasNullDefaultValue())
			{
				fieldValue.reset(new FieldValue(field));
			}
			else
			{
				FieldTypes fieldType = field.getType();
				switch(fieldType)
				{
					case BOOLEAN:
						fieldValue.reset(new FieldValue(field, field.getBooleanDefaultValue()));
						break;

					case INT:
						fieldValue.reset(new FieldValue(field, field.getIntDefaultValue()));
						break;

					case DOUBLE:
						fieldValue.reset(new FieldValue(field, field.getDoubleDefaultValue()));
						break;

					case STRING:
						fieldValue.reset(new FieldValue(field, field.getStringDefaultValue()));
						break;

					case DATETIME:
						fieldValue.reset(new FieldValue(field, field.getDateTimeDefaultValue()));
						break;

					case BINARY:
						throw std::runtime_error( "Binary fields can't belong to primary key." );
						break;

					default:
						throw std::runtime_error( "Unknown field type." );
				}
			}

			m_fieldValues.push_back(std::move(fieldValue));
		}
	}

	ITable& PrimaryKeyValue::getTable() const
	{
		return m_primaryKey.getTable();
	}

	const IPrimaryKey& PrimaryKeyValue::getPrimaryKey() const
	{
		return m_primaryKey;
	}

	unsigned int PrimaryKeyValue::getFieldValuesCount() const
	{
		return static_cast<unsigned int>(m_fieldValues.size());
	}

	IFieldValue& PrimaryKeyValue::getFieldValue(unsigned int index) const
	{
		if (index >= m_fieldValues.size())
		{
			throw std::runtime_error("Invalid primary key field index");

		}

		return *(m_fieldValues.at(index));
	}

	IFieldValue& PrimaryKeyValue::getFieldValue(const std::string& fieldName) const
	{
		const auto fieldValue = std::find_if(m_fieldValues.cbegin(), m_fieldValues.cend(), 
			[&fieldName] (const std::unique_ptr<IFieldValue>& field) 
			{ 
				return field->getField().getName() == fieldName;
			});

		if (fieldValue != m_fieldValues.cend())
		{
			return *(fieldValue->get());
		}

		throw std::runtime_error( "The requested primary key field doesn't exist" );
	}
}