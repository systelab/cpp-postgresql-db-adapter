#pragma once

#include "DbAdapterInterface/IPrimaryKeyValue.h"

namespace systelab { namespace db { namespace postgresql {

	class PrimaryKeyValue : public IPrimaryKeyValue
	{
	public:
		PrimaryKeyValue(const IPrimaryKey& primaryKey);
		~PrimaryKeyValue() override = default;

		ITable& getTable() const override;
		const IPrimaryKey& getPrimaryKey() const override;

		unsigned int getFieldValuesCount() const override;
		IFieldValue& getFieldValue(unsigned int index) const override;
		IFieldValue& getFieldValue(const std::string& fieldName) const override;

	private:
		const IPrimaryKey& m_primaryKey;
		std::vector<std::unique_ptr<IFieldValue>> m_fieldValues;
	};
}}}