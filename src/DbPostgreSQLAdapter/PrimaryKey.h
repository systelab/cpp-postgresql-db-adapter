#pragma once

#include "DbAdapterInterface/IPrimaryKey.h"

namespace systelab {
	namespace db {
		class IField;
		class ITable;
	}
}

namespace systelab { namespace db { namespace postgresql {

	class PrimaryKey : public IPrimaryKey
	{
	public:
		PrimaryKey(ITable& table);
		~PrimaryKey() override = default;

		ITable& getTable() const override;

		unsigned int getFieldsCount() const override;
		const IField& getField(unsigned int index) const override;
		const IField& getField(const std::string& fieldName) const override;

	private:
		ITable& m_table;
		std::vector<const IField*> m_fields;
	};
}}}