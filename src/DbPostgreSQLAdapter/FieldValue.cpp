#include "stdafx.h"
#include "FieldValue.h"

#include "DbAdapterInterface/IBinaryValue.h"
#include "PostgresUtils.h"

namespace systelab::db::postgresql {

	FieldValue::FieldValue(const IField& field)
		: m_field(field)
		, m_nullValue(true)
		, m_default(false)
		, m_boolValue(false)
		, m_intValue(0)
		, m_doubleValue(0.)
		, m_stringValue("")
		, m_dateTimeValue()
	{
	}

	FieldValue::FieldValue(const IField& field, bool value)
		: m_field(field)
		, m_nullValue(false)
		, m_default(false)
		, m_boolValue(value)
		, m_intValue(0)
		, m_doubleValue(0.)
		, m_stringValue("")
		, m_dateTimeValue()
	{
		if (m_field.getType() != BOOLEAN)
		{
			throw std::runtime_error("Field doesn't accept a boolean value");
		}
	}

	FieldValue::FieldValue(const IField& field, int value)
		: m_field(field)
		, m_nullValue(false)
		, m_default(false)
		, m_boolValue(false)
		, m_intValue(value)
		, m_doubleValue(0.)
		, m_stringValue("")
		, m_dateTimeValue()
	{
		if (m_field.getType() != INT)
		{
			throw std::runtime_error("Field doesn't accept an integer value");
		}
	}

	FieldValue::FieldValue(const IField& field, double value)
		: m_field(field)
		, m_nullValue(false)
		, m_default(false)
		, m_boolValue(false)
		, m_intValue(0)
		, m_doubleValue(value)
		, m_stringValue("")
		, m_dateTimeValue()
	{
		if (m_field.getType() != DOUBLE)
		{
			throw std::runtime_error("Field doesn't accept a double value");
		}
	}

	FieldValue::FieldValue(const IField& field, const std::string& value)
		: m_field(field)
		, m_nullValue(false)
		, m_default(false)
		, m_boolValue(false)
		, m_intValue(0)
		, m_doubleValue(0.)
		, m_stringValue(value)
		, m_dateTimeValue()
	{
		if (m_field.getType() != STRING)
		{
			throw std::runtime_error("Field doesn't accept a string value");
		}
	}

	FieldValue::FieldValue(const IField& field, const std::chrono::system_clock::time_point& value)
		: m_field(field)
		, m_nullValue(true)
		, m_default(false)
		, m_boolValue(false)
		, m_intValue(0)
		, m_doubleValue(0.)
		, m_stringValue("")
		, m_dateTimeValue()
	{
		if (m_field.getType() != DATETIME)
		{
			throw std::runtime_error("Field doesn't accept a datetime value");
		}

		if (!utils::isDateTimeNull(value))
		{
			m_dateTimeValue = value;
			m_nullValue = false;
		}
	}

	FieldValue::~FieldValue() = default;

	const IField& FieldValue::getField() const
	{
		return m_field;
	}

	bool FieldValue::isNull() const
	{
		return m_nullValue;
	}

	bool FieldValue::isDefault() const
	{
		return m_default;
	}

	bool FieldValue::getBooleanValue() const
	{
		if (isNull())
		{
			throw std::runtime_error("Field value is null");
		}

		if (isDefault())
		{
			throw std::runtime_error("Field value is default");
		}

		if (m_field.getType() != BOOLEAN)
		{
			throw std::runtime_error("Field type isn't boolean");
		}

		return m_boolValue;
	}

	int FieldValue::getIntValue() const
	{
		if (isNull())
		{
			throw std::runtime_error("Field value is null");
		}

		if (isDefault())
		{
			throw std::runtime_error("Field value is default");
		}

		if (m_field.getType() != INT)
		{
			throw std::runtime_error("Field type isn't integer");
		}

		return m_intValue;
	}

	double FieldValue::getDoubleValue() const
	{
		if (isNull())
		{
			throw std::runtime_error("Field value is null");
		}

		if (isDefault())
		{
			throw std::runtime_error("Field value is default");
		}

		if (m_field.getType() != DOUBLE)
		{
			throw std::runtime_error("Field type isn't double");
		}

		return m_doubleValue;
	}

	std::string FieldValue::getStringValue() const
	{
		if (isNull())
		{
			throw std::runtime_error("Field value is null");
		}

		if (isDefault())
		{
			throw std::runtime_error("Field value is default");
		}

		if (m_field.getType() != STRING)
		{
			throw std::runtime_error("Field type isn't string");
		}

		return m_stringValue;
	}

	std::chrono::system_clock::time_point FieldValue::getDateTimeValue() const
	{
		if (isDefault())
		{
			throw std::runtime_error("Field value is default");
		}

		if (m_field.getType() == DATETIME)
		{
			return m_dateTimeValue;
		}

		if (m_field.getType() != STRING)
		{
			throw std::runtime_error("Field type isn't datetime");
		}

		if (!isNull())
		{
			return utils::stringISOToDateTime(m_stringValue);
		}
		
		return std::chrono::system_clock::time_point {};
	}

	IBinaryValue& FieldValue::getBinaryValue() const
	{
		throw std::runtime_error("Not implemented yet");
	}

	void FieldValue::setValue(const IFieldValue& srcFieldValue)
	{
		const FieldTypes srcFieldType = srcFieldValue.getField().getType();
		if (srcFieldType != getField().getType())
		{
			throw std::runtime_error("Can't set the value of a field of another type");
		}

		if (srcFieldValue.isNull())
		{
			setNull();
		}
		else if (srcFieldValue.isDefault())
		{
			setDefault();
		}
		else
		{
			switch (srcFieldType)
			{
				case BOOLEAN:
					setBooleanValue(srcFieldValue.getBooleanValue());
					break;
				case INT:
					setIntValue(srcFieldValue.getIntValue());
					break;
				case DOUBLE:
					setDoubleValue(srcFieldValue.getDoubleValue());
					break;
				case STRING:
					setStringValue(srcFieldValue.getStringValue());
					break;
				case DATETIME:
					setDateTimeValue(srcFieldValue.getDateTimeValue());
					break;
				case BINARY:
					throw std::runtime_error("Invalid record field type.");
					break;
			}
		}
	}

	void FieldValue::setNull()
	{
		m_nullValue = true;
		m_default = false;
		m_boolValue = false;
		m_intValue = 0;
		m_doubleValue = 0.;
		m_stringValue = "";
		m_dateTimeValue = std::chrono::system_clock::time_point {};
	}

	void FieldValue::setDefault()
	{
		m_nullValue = false;
		m_default = true;
		m_boolValue = false;
		m_intValue = 0;
		m_doubleValue = 0.;
		m_stringValue = "";
		m_dateTimeValue = std::chrono::system_clock::time_point {};
	}

	void FieldValue::setBooleanValue(bool value)
	{
		if (m_field.getType() != BOOLEAN)
		{
			throw std::runtime_error("Field type isn't boolean");
		}

		m_boolValue = value;
		m_nullValue = false;
		m_default = false;
	}

	void FieldValue::setIntValue(int value)
	{
		if (m_field.getType() != INT)
		{
			throw std::runtime_error("Field type isn't integer");
		}
		
		m_intValue = value;
		m_nullValue = false;
		m_default = false;
	}

	void FieldValue::setDoubleValue(double value)
	{
		if (m_field.getType() != DOUBLE)
		{
			throw std::runtime_error("Field type isn't double");
		}

		m_doubleValue = value;
		m_nullValue = false;
		m_default = false;
	}

	void FieldValue::setStringValue(const std::string& value)
	{
		if (m_field.getType() != STRING)
		{
			throw std::runtime_error("Field type isn't string");
		}
		
		m_stringValue = value;
		m_nullValue = false;
		m_default = false;
	}

	void FieldValue::setDateTimeValue(const std::chrono::system_clock::time_point& value)
	{
		if (m_field.getType() != DATETIME)
		{
			throw std::runtime_error("Field type isn't datetime");
		}
		
		m_dateTimeValue = value;
		m_nullValue = utils::isDateTimeNull(value);
		m_default = false;
	}

	void FieldValue::setBinaryValue(std::unique_ptr<IBinaryValue> value)
	{
		throw std::runtime_error("Not implemented yet");
	}

	void FieldValue::useDefaultValue()
	{
		const FieldTypes fieldType = m_field.getType();
		switch (fieldType)
		{
			case BOOLEAN:
				setBooleanValue(m_field.getBooleanDefaultValue());
				break;

			case INT:
				setIntValue(m_field.getIntDefaultValue());
				break;

			case DOUBLE:
				setDoubleValue(m_field.getDoubleDefaultValue());
				break;

			case STRING:
				setStringValue(m_field.getStringDefaultValue());
				break;

			case DATETIME:
				setDateTimeValue(m_field.getDateTimeDefaultValue());
				break;

			case BINARY:
			default:
				throw std::runtime_error("Invalid field type.");
				break;
		}
	}

	std::unique_ptr<IFieldValue> FieldValue::clone() const
	{
		if (isNull())
		{
			return std::unique_ptr<IFieldValue>(new FieldValue(m_field));
		}
		
		if (isDefault())
		{
			std::unique_ptr<IFieldValue> fieldValue(new FieldValue(m_field));
			fieldValue->setDefault();
			return fieldValue;
		}

		const FieldTypes fieldType = m_field.getType();
		switch (fieldType)
		{
			case BOOLEAN:
				return std::make_unique<FieldValue>(m_field, m_boolValue);

			case INT:
				return std::make_unique<FieldValue>(m_field, m_intValue);

			case DOUBLE:
				return std::make_unique<FieldValue>(m_field, m_doubleValue);

			case STRING:
				return std::make_unique<FieldValue>(m_field, m_stringValue);

			case DATETIME:
				return std::make_unique<FieldValue>(m_field, m_dateTimeValue);

			case BINARY:
			default:
				throw std::runtime_error("Invalid field type.");
				break;
		}
	}
}