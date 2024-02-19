#pragma once

#include "DbAdapterInterface/IFieldValue.h"

namespace systelab::db {
	class IBinaryValue;
	class IField;
}

namespace systelab::db::postgresql {

	class FieldValue : public IFieldValue
	{
	public:

		FieldValue(const IField&);
		FieldValue(const IField&, bool);
		FieldValue(const IField&, int);
		FieldValue(const IField&, double);
		FieldValue(const IField&, const std::string&);
		FieldValue(const IField&, const std::chrono::system_clock::time_point&);
		~FieldValue(void) override;

		const IField& getField() const override;

		bool isNull() const override;
		bool isDefault() const override;
		bool getBooleanValue() const override;
		int getIntValue() const override;
		double getDoubleValue() const override;
		std::string getStringValue() const override;
		std::chrono::system_clock::time_point getDateTimeValue() const override;
		IBinaryValue& getBinaryValue() const override;

		void setValue(const IFieldValue&) override;
		void setNull() override;
		void setDefault();
		void setBooleanValue(bool value) override;
		void setIntValue(int value) override;
		void setDoubleValue(double value) override;
		void setStringValue(const std::string& value) override;
		void setDateTimeValue(const std::chrono::system_clock::time_point& value) override;
		void setBinaryValue(std::unique_ptr<IBinaryValue> value) override;

		void useDefaultValue();

		std::unique_ptr<IFieldValue> clone() const;

	private:
		const IField& m_field;

		bool m_nullValue;
		bool m_default;
		bool m_boolValue;
		int m_intValue;
		double m_doubleValue;
		std::string m_stringValue;
		std::chrono::system_clock::time_point m_dateTimeValue;
	};
}