#include "stdafx.h"
#include "Field.h"

namespace {
	std::chrono::system_clock::time_point stringISOToDateTime(const std::string& dateTime)
	{
		std::chrono::system_clock::time_point timePointDateTime;
		std::istringstream{ dateTime } >> std::chrono::parse("%F %T%z", timePointDateTime);
		return timePointDateTime;
	}
}

namespace systelab::db::postgresql {

	Field::Field(const unsigned int index,
				 const std::string& name,
				 const FieldTypes type,
				 const std::string& defaultValue,
				 const bool primaryKey)
		: m_index(index)
		, m_name(name)
		, m_type(type)
		, m_primaryKey(primaryKey)
	{
		setDefaultValue(type, defaultValue);
	}

	unsigned int Field::getIndex() const
	{
		return m_index;
	}

	std::string Field::getName() const
	{
		return m_name;
	}

	FieldTypes Field::getType() const
	{
		return m_type;
	}

	bool Field::hasNullDefaultValue() const
	{
		return m_nullDefaultValue;
	}

	bool Field::getBooleanDefaultValue() const
	{
		if (hasNullDefaultValue())
		{
			throw std::runtime_error("Default value is null");
		}

		if (m_type != BOOLEAN)
		{
			throw std::runtime_error("Field type isn't boolean");
		}

		return m_defaultBoolValue;
	}

	int Field::getIntDefaultValue() const
	{
		if (hasNullDefaultValue())
		{
			throw std::runtime_error("Default value is null");
		}

		if (m_type != INT)
		{
			throw std::runtime_error("Field type isn't integer");
		}

		return m_defaultIntValue;
	}

	double Field::getDoubleDefaultValue() const
	{
		if (hasNullDefaultValue())
		{
			throw std::runtime_error("Default value is null");
		}

		if (m_type != DOUBLE)
		{
			throw std::runtime_error("Field type isn't double");
		}

		return m_defaultDoubleValue;
	}

	std::string Field::getStringDefaultValue() const
	{
		if (hasNullDefaultValue())
		{
			throw std::runtime_error("Default value is null");
		}

		if (m_type != STRING)
		{
			throw std::runtime_error("Field type isn't string");
		}

		return m_defaultStringValue;
	}

	std::chrono::system_clock::time_point Field::getDateTimeDefaultValue() const
	{
		if (hasNullDefaultValue())
		{
			throw std::runtime_error("Default value is null");
		}

		if (m_type != DATETIME)
		{
			throw std::runtime_error("Field type isn't datetime");
		}

		return m_defaultDateTimeValue;
	}

	IBinaryValue& Field::getBinaryDefaultValue() const
	{
		if (hasNullDefaultValue())
		{
			throw std::runtime_error("Default value is null");
		}

		if (m_type != BINARY)
		{
			throw std::runtime_error("Field type isn't binary");
		}

		return *m_defaultBinaryValue.get();
	}

	bool Field::isPrimaryKey() const
	{
		return m_primaryKey;
	}

	void Field::setDefaultValue(FieldTypes type, const std::string& defaultValue)
	{
		m_defaultBoolValue = false;
		m_defaultIntValue = 0;
		m_defaultDoubleValue = 0.;
		m_defaultStringValue = "";
		m_defaultDateTimeValue = std::chrono::system_clock::time_point {};
		m_defaultBinaryValue.reset();

		std::string defaultValueUpper = defaultValue;
		std::transform(defaultValueUpper.begin(), defaultValueUpper.end(), defaultValueUpper.begin(), ::toupper);
		if (defaultValue.empty() || defaultValueUpper == "NULL")
		{
			m_nullDefaultValue = true;
		}
		else
		{
			m_nullDefaultValue = false;
			switch (type)
			{
				case BOOLEAN:
					m_defaultBoolValue = (defaultValueUpper == "t");
					break;
				case INT:
					m_defaultIntValue = std::stoi(defaultValue);
					break;
				case DOUBLE:
					m_defaultDoubleValue = std::stod(defaultValue);
					break;
				case STRING:
					m_defaultStringValue = defaultValue;
					break;
				case DATETIME:
					m_defaultDateTimeValue = stringISOToDateTime(defaultValue);
					break;
				case BINARY:
				default:
					throw std::runtime_error("Invalid record field type." );
					break;
			}
		}
	}
}