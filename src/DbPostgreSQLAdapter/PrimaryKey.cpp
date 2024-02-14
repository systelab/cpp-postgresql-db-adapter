#include "stdafx.h"
#include "PrimaryKey.h"

#include "DbAdapterInterface/ITable.h"


namespace systelab { namespace db { namespace postgresql {

	PrimaryKey::PrimaryKey(ITable& table)
		:m_table(table)
	{
		const unsigned int tableFieldsCount = m_table.getFieldsCount();
		for (unsigned int i = 0; i < tableFieldsCount; i++)
		{
			const IField& tableField = m_table.getField(i);
			if (tableField.isPrimaryKey())
			{
				m_fields.push_back(&tableField);
			}
		}
	}

	ITable& PrimaryKey::getTable() const
	{
		return m_table;
	}

	unsigned int PrimaryKey::getFieldsCount() const
	{
		return static_cast<unsigned int>(m_fields.size());
	}

	const IField& PrimaryKey::getField(unsigned int index) const
	{
		if (index >= m_fields.size())
		{
			throw std::runtime_error( "Invalid primary key field index" );
		}

		return *(m_fields.at(index));
	}

	const IField& PrimaryKey::getField(const std::string& fieldName) const
	{
		const auto fieldIterator = std::find_if(m_fields.cbegin(), m_fields.cend(), 
			[&fieldName](const IField* field)
			{ 
				return field->getName() == fieldName; 
			});
		
		if (fieldIterator != m_fields.end())
		{
			return **fieldIterator;
		}

		throw std::runtime_error( "The requested primary key field doesn't exist" );
	}
}}}