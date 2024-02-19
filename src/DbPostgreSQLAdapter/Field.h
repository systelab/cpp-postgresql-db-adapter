#pragma once

#include "DbAdapterInterface/IField.h"

namespace systelab::db::postgresql {

	class Field : public IField
	{
	public:
		Field(const unsigned int index, const std::string& name, const FieldTypes type, const std::string& defaultValue, const bool primaryKey);
		~Field() override = default;

		unsigned int getIndex() const override;
		std::string getName() const override;
		FieldTypes getType() const override;
		bool isPrimaryKey() const override;

		bool hasNullDefaultValue() const override;
		bool getBooleanDefaultValue() const override;
		int getIntDefaultValue() const override;
		double getDoubleDefaultValue() const override;
		std::string getStringDefaultValue() const override;
		std::chrono::system_clock::time_point getDateTimeDefaultValue() const override;
		IBinaryValue& getBinaryDefaultValue() const override;

	private:
		unsigned int m_index;
		std::string m_name;
		FieldTypes m_type;
		bool m_primaryKey;

		bool m_nullDefaultValue;
		bool m_defaultBoolValue;
		int m_defaultIntValue;
		double m_defaultDoubleValue;
		std::string m_defaultStringValue;
		std::chrono::system_clock::time_point m_defaultDateTimeValue;

		void setDefaultValue(FieldTypes type, const std::string& defaultValue);
	};
}