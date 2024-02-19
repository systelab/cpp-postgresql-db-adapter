#include "stdafx.h"
#include "Table.h"

#include "Database.h"
#include "Field.h"
#include "FieldValue.h"
#include "PrimaryKey.h"
#include "PrimaryKeyValue.h"
#include "TableRecord.h"

#include <DbAdapterInterface/IRecordSet.h>
#include <DbAdapterInterface/IRecord.h>

namespace {
	std::string dateTimeToISOString(const std::chrono::system_clock::time_point& dateTime)
	{
		if (dateTime != std::chrono::system_clock::time_point{})
		{
			return std::format("{:%F %T%z}", dateTime);
		}

		return "";
	}

	std::string getSQLValue(const systelab::db::IFieldValue& fieldValue, bool forComparison, bool forAssignment)
	{
		std::ostringstream fieldValueStream;
		if (fieldValue.isNull())
		{
			if (forComparison)
			{
				fieldValueStream << " IS ";
			}
			else if (forAssignment)
			{
				fieldValueStream << " = ";
			}

			fieldValueStream << "NULL";
		}
		else
		{
			if (forComparison || forAssignment)
			{
				fieldValueStream << " = ";
			}

			const systelab::db::FieldTypes fieldType = fieldValue.getField().getType();
			switch (fieldType)
			{
			case systelab::db::BOOLEAN:
				fieldValueStream << (fieldValue.getBooleanValue() ? "True" : "False");
				break;
			case systelab::db::INT:
				fieldValueStream << fieldValue.getIntValue();
				break;
			case systelab::db::DOUBLE:
				fieldValueStream << std::fixed << std::setprecision(10) << fieldValue.getDoubleValue();
				break;
			case systelab::db::STRING:
				fieldValueStream << "'" << fieldValue.getStringValue() << "'";
				break;
			case systelab::db::DATETIME:
				fieldValueStream << "'" << dateTimeToISOString(fieldValue.getDateTimeValue()) << "'";
				break;
			case systelab::db::BINARY:
				throw std::runtime_error("Insert of tables with binary fields not implemented.");
				break;
			default:
				throw std::runtime_error("Invalid record field type.");
				break;
			}
		}

		return fieldValueStream.str();
	}

	std::string getStringList(const std::vector<std::string>& items, const std::string& separator)
	{
		std::string stringList = "";
		if (!items.empty())
		{
			stringList = std::accumulate(items.begin()+1, items.end(), items.at(0),
				[&separator](const std::string& a, const std::string& b)
				{
					return a + separator + b;
				});
		}

		return stringList;
	}

	systelab::db::FieldTypes getTypeFromPostgresTypeName(std::string postgresTypeName)
	{
		if (postgresTypeName == "boolean")
		{
			return systelab::db::BOOLEAN;
		}
		else if (postgresTypeName == "integer")
		{
			return systelab::db::INT;
		}
		else if (postgresTypeName == "real")
		{
			return systelab::db::DOUBLE;
		}
		else if (postgresTypeName == "text" ||
				 postgresTypeName == "character" ||
			     postgresTypeName == "character varying")
		{
			return systelab::db::STRING;
		}
		else if (postgresTypeName == "timestamp with time zone")
		{
			return systelab::db::DATETIME;
		}
		else
		{
			std::string excMessage = "Postgress type name not recognized: " + postgresTypeName;
			throw std::runtime_error(excMessage.c_str());
		}
	}
}

namespace systelab::db::postgresql {

	Table::Table(Database& database, const std::string& name)
		: m_database(database)
		, m_name(name)
	{
		loadFields();
		m_primaryKey = std::make_unique<PrimaryKey>(*this);
	}

	std::string Table::getName() const
	{
		return m_name;
	}

	const IPrimaryKey& Table::getPrimaryKey() const
	{
		return *(m_primaryKey.get());
	}

	unsigned int Table::getFieldsCount() const
	{
		return static_cast<unsigned int>(m_fields.size());
	}

	const IField& Table::getField(unsigned int index) const
	{
		return *(m_fields.at(index).get());
	}

	const IField& Table::getField(const std::string& fieldName) const
	{
		const auto field = std::find_if(m_fields.cbegin(), m_fields.cend(),
			[&fieldName](const auto& field)
			{
				return field->getName() == fieldName;
			});

		if (field == m_fields.cend())
		{
			throw std::runtime_error("The requested field doesn't exist");
		}

		return *(field->get());
		
	}

	std::unique_ptr<IFieldValue> Table::createFieldValue(const IField& field) const
	{
		return std::unique_ptr<IFieldValue>(new FieldValue(field));
	}

	std::unique_ptr<IFieldValue> Table::createFieldValue(const IField& field, bool value) const
	{
		return std::unique_ptr<IFieldValue>(new FieldValue(field, value));
	}

	std::unique_ptr<IFieldValue> Table::createFieldValue(const IField& field, int value) const
	{
		return std::unique_ptr<IFieldValue>(new FieldValue(field,value));
	}

	std::unique_ptr<IFieldValue> Table::createFieldValue(const IField& field, double value) const
	{
		return std::unique_ptr<IFieldValue>(new FieldValue(field,value));
	}

	std::unique_ptr<IFieldValue> Table::createFieldValue(const IField& field, const std::string& value) const
	{
		return std::unique_ptr<IFieldValue>(new FieldValue(field,value));
	}

	std::unique_ptr<IFieldValue> Table::createFieldValue(const IField& field, const std::chrono::system_clock::time_point& value) const
	{
		return std::unique_ptr<IFieldValue>(new FieldValue(field,value));
	}

	std::unique_ptr<IFieldValue> Table::createFieldValue(const IField& field, std::unique_ptr<IBinaryValue> value) const
	{
		throw std::runtime_error("Not implemented yet");
	}

	std::unique_ptr<IPrimaryKeyValue> Table::createPrimaryKeyValue() const
	{
		const IPrimaryKey& primaryKey = getPrimaryKey();
		return std::unique_ptr<IPrimaryKeyValue>(new PrimaryKeyValue(primaryKey));
	}

	std::unique_ptr<ITableRecordSet> Table::getAllRecords() const
	{
		std::string query = "SELECT * FROM " + m_name;
		return m_database.executeTableQuery(query, const_cast<Table&>(*this));
	}

	std::unique_ptr<ITableRecord> Table::getRecordByPrimaryKey(const IPrimaryKeyValue& primaryKeyValue) const
	{
		std::vector<IFieldValue*> conditionValues;
		const unsigned int primaryKeyFieldValuesCount = primaryKeyValue.getFieldValuesCount();
		for (unsigned int i = 0; i < primaryKeyFieldValuesCount; i++)
		{
			IFieldValue& fieldValue = primaryKeyValue.getFieldValue(i);
			conditionValues.push_back(&fieldValue);
		}

		std::unique_ptr<ITableRecordSet> recordset = filterRecordsByFields(conditionValues);
		if (recordset->getRecordsCount() > 0)
		{
			return recordset->copyCurrentRecord();
		}
		else
		{
			return std::unique_ptr<ITableRecord>();
		}
	}

	std::unique_ptr<ITableRecordSet> Table::filterRecordsByField(const IFieldValue& conditionValue, const IField* orderByField) const
	{
		std::vector<IFieldValue*> conditionValues;
		conditionValues.push_back(&const_cast<IFieldValue&>(conditionValue));
		return filterRecordsByFields(conditionValues, orderByField);
	}

	std::unique_ptr<ITableRecordSet> Table::filterRecordsByFields(const std::vector<IFieldValue*>& conditionValues, const IField* orderByField) const
	{
		std::vector<std::string> conditionValuesSQL;
		unsigned int nConditionFieldValues = (unsigned int) conditionValues.size();
		for (unsigned int j = 0; j < nConditionFieldValues; j++)
		{
			IFieldValue& conditionFieldValue = *(conditionValues[j]);
			const IField& field = conditionFieldValue.getField();

			if (!isOwned(field))
			{
				throw std::runtime_error("Can't filter by fields that don't come from this table.");
			}

			if (!conditionFieldValue.isDefault())
			{
				std::string conditionFieldValueName = field.getName();
				std::string conditionFieldValueSQLValue = getSQLValue(conditionFieldValue, true, false);
				conditionValuesSQL.push_back( conditionFieldValueName + conditionFieldValueSQLValue );
			}
		}

		std::string conditionSQLStr = getStringList(conditionValuesSQL, " AND ");

		if (orderByField)
		{
			conditionSQLStr += " ORDER BY " + orderByField->getName();
		}

		return filterRecordsByCondition(conditionSQLStr);
	}

	std::unique_ptr<ITableRecordSet> Table::filterRecordsByCondition(const std::string& SQLCondition) const
	{
		const std::string query = "SELECT * FROM " + m_name + " WHERE " + SQLCondition;
		return m_database.executeTableQuery(query, const_cast<Table&>(*this));
	}

	int Table::getMaxFieldValueInt(const IField& field) const
	{
		const std::string query = "SELECT MAX(" + field.getName() + ") FROM " + m_name;
		std::unique_ptr<IRecordSet> result = m_database.executeQuery(query);
		if (result->getRecordsCount() != 1)
		{
			throw "Table::getMaxFieldValueInt returned an invalid amount of results";
		}
		return result->getCurrentRecord().getFieldValue(0).getIntValue();
	}

	std::unique_ptr<ITableRecord> Table::createRecord() const
	{
		std::vector< std::unique_ptr<IFieldValue> > fieldValues;

		const unsigned int fieldsCount = static_cast<unsigned int>(m_fields.size());
		for (unsigned int i = 0; i < fieldsCount; i++)
		{
			const IField& field = *(m_fields.at(i).get());
			std::unique_ptr<IFieldValue> fieldValue = createFieldValue(field);
			fieldValue->setDefault();
			fieldValues.push_back(std::move(fieldValue));
		}

		return std::unique_ptr<ITableRecord>(new TableRecord(const_cast<Table&>(*this), fieldValues));
	}

	std::unique_ptr<ITableRecord> Table::copyRecord(const ITableRecord& record) const
	{
		if (&record.getTable() != this)
		{
			throw std::runtime_error("Can't copy records from other tables." );
		}

		std::vector< std::unique_ptr<IFieldValue> > copyFieldValues;
		const unsigned int fieldsValuesCount = record.getFieldValuesCount();
		for (unsigned int i = 0; i < fieldsValuesCount; i++)
		{
			const IFieldValue& fieldValue = record.getFieldValue(i);
			std::unique_ptr<IFieldValue> copyFieldValue = fieldValue.clone();
			copyFieldValues.push_back(std::move(copyFieldValue));
		}

		return std::unique_ptr<ITableRecord>(new TableRecord(const_cast<Table&>(*this), copyFieldValues));
	}

	RowsAffected Table::insertRecord(ITableRecord& record)
	{
		if (&record.getTable() != this)
		{
			throw std::runtime_error("Can't insert records from other tables." );
		}

		std::vector<std::string> fieldNamesSQL;
		std::vector<std::string> fieldValuesSQL;
		std::string primaryKey;
		const unsigned int fieldsValuesCount = record.getFieldValuesCount();
		for (unsigned int i= 0; i < fieldsValuesCount; i++)
		{
			const IFieldValue& fieldValue = record.getFieldValue(i);
			const IField& field = fieldValue.getField();
			if (!fieldValue.isDefault())
			{
				std::string fieldName = fieldValue.getField().getName();
				fieldNamesSQL.push_back(fieldName);

				std::string fieldValueSQL = getSQLValue(fieldValue, false, false);
				fieldValuesSQL.push_back(fieldValueSQL);
			}

			if (field.isPrimaryKey())
			{
				primaryKey = fieldValue.getField().getName();
			}
		}

		std::string fieldNamesSQLStr = getStringList(fieldNamesSQL, ",");
		std::string fieldValuesSQLStr = getStringList(fieldValuesSQL, ",");
		std::string insert = "INSERT INTO " + m_name + 
							 " (" + fieldNamesSQLStr + ") " +
							 " VALUES (" + fieldValuesSQLStr + ") RETURNING " + primaryKey;

		m_database.executeOperation(insert);
		RowsAffected rows = m_database.getRowsAffectedByLastChangeOperation();

		if (rows > 0)
		{
			for (unsigned int j = 0; j < fieldsValuesCount; j++)
			{
				IFieldValue& fieldValue = record.getFieldValue(j);
				if (fieldValue.isDefault())
				{
					const IField& field = fieldValue.getField();
					if (field.isPrimaryKey())
					{
						RowId rowId = m_database.getLastInsertedRowId();
						fieldValue.setIntValue(rowId);
					}
					else
					{
						fieldValue.useDefaultValue();
					}
				}
			}
		}

		return rows;
	}

	RowsAffected Table::updateRecord(const ITableRecord& record)
	{
		if (&record.getTable() != this)
		{
			throw std::runtime_error("Can't update records from other tables." );
		}

		std::unique_ptr<IPrimaryKeyValue> primaryKeyValue = createPrimaryKeyValue();
		unsigned int nPrimaryKeyFieldValues = primaryKeyValue->getFieldValuesCount();
		for(unsigned int i = 0; i < nPrimaryKeyFieldValues; i++)
		{
			IFieldValue& primaryKeyFieldValue = primaryKeyValue->getFieldValue(i);
			const IField& primaryKeyField = primaryKeyFieldValue.getField();

			std::string fieldName = primaryKeyField.getName();
			IFieldValue& recordFieldValue = record.getFieldValue(fieldName);
			primaryKeyFieldValue.setValue(recordFieldValue);
		}

		std::vector<IFieldValue*> newValues;
		unsigned int nRecordFieldValues = record.getFieldValuesCount();
		for(unsigned int i = 0; i < nRecordFieldValues; i++)
		{
			IFieldValue& recordFieldValue = record.getFieldValue(i);
			const IField& recordField = recordFieldValue.getField();
			if (!recordField.isPrimaryKey())
			{
				newValues.push_back(&recordFieldValue);
			}
		}

		return updateRecord(newValues, *primaryKeyValue);
	}

	RowsAffected Table::updateRecord(const std::vector<IFieldValue*>& newValues, const IPrimaryKeyValue& primaryKeyValue)
	{
		if (&primaryKeyValue.getTable() != this)
		{
			throw std::runtime_error( "Can't update records using a primary key value from another table." );
		}

		std::vector<IFieldValue*> conditionValues;
		unsigned int nPrimaryKeyFieldValues = primaryKeyValue.getFieldValuesCount();
		for (unsigned int i = 0; i < nPrimaryKeyFieldValues; i++)
		{
			conditionValues.push_back( &primaryKeyValue.getFieldValue(i) );
		}

		return updateRecordsByCondition(newValues, conditionValues);
	}

	RowsAffected Table::deleteRecord(const ITableRecord& record)
	{
		if (&record.getTable() != this)
		{
			throw std::runtime_error("Can't delete records from other tables." );
		}

		std::vector<IFieldValue*> conditionValues;
		unsigned int nFieldValues = record.getFieldValuesCount();
		for(unsigned int i = 0; i < nFieldValues; i++)
		{
			IFieldValue& fieldValue = record.getFieldValue(i);
			if (fieldValue.getField().isPrimaryKey())
			{
				conditionValues.push_back(&fieldValue);
			}
		}

		return deleteRecordsByCondition(conditionValues);
	}

	RowsAffected Table::deleteRecord(const IPrimaryKeyValue& primaryKeyValue)
	{
		if (&primaryKeyValue.getTable() != this)
		{
			throw std::runtime_error( "Can't delete records using a primary key value from another table." );
		}

		std::vector<IFieldValue*> conditionValues;
		unsigned int nPrimaryKeyFieldValues = primaryKeyValue.getFieldValuesCount();
		for (unsigned int i = 0; i < nPrimaryKeyFieldValues; i++)
		{
			conditionValues.push_back( &primaryKeyValue.getFieldValue(i) );
		}

		return deleteRecordsByCondition(conditionValues);
	}

	RowsAffected Table::updateRecordsByCondition(const std::vector<IFieldValue*>& newValues, const std::vector<IFieldValue*>& conditionValues)
	{
		std::vector<std::string> newValuesSQL;
		unsigned int nNewFieldValues = (unsigned int) newValues.size();
		for (unsigned int i = 0; i < nNewFieldValues; i++)
		{
			IFieldValue& newFieldValue = *(newValues[i]);
			const IField& field = newFieldValue.getField();

			if (!isOwned(field))
			{
				throw std::runtime_error("Can't update records using new values that aren't owned by this table." );
			}

			if (!newFieldValue.isDefault())
			{
				std::string newFieldValueName = field.getName();
				std::string newFieldValueSQLValue = getSQLValue(newFieldValue, false, true);
				newValuesSQL.push_back( newFieldValueName + newFieldValueSQLValue );
			}
		}

		std::vector<std::string> conditionValuesSQL;
		unsigned int nConditionFieldValues = (unsigned int) conditionValues.size();
		for (unsigned int j = 0; j < nConditionFieldValues; j++)
		{
			IFieldValue& conditionFieldValue = *(conditionValues[j]);
			const IField& field = conditionFieldValue.getField();

			if (!isOwned(field))
			{
				throw std::runtime_error("Can't update records using condition values that aren't owned by this table." );
			}

			if (!conditionFieldValue.isDefault())
			{
				std::string conditionFieldValueName = field.getName();
				std::string conditionFieldValueSQLValue = getSQLValue(conditionFieldValue, true, false);
				conditionValuesSQL.push_back( conditionFieldValueName + conditionFieldValueSQLValue );
			}
		}

		if (!newValuesSQL.empty() && !conditionValuesSQL.empty())
		{
			std::string newValuesSQLStr = getStringList(newValuesSQL, ", ");
			std::string conditionSQLStr = getStringList(conditionValuesSQL, " AND ");

			std::string update = "UPDATE " + m_name + " " +
								 "SET " + newValuesSQLStr + " " +
								 "WHERE " + conditionSQLStr + ";";

			m_database.executeOperation(update);
			return m_database.getRowsAffectedByLastChangeOperation();
		}
		else
		{
			return (RowsAffected) 0;
		}
	}

	RowsAffected Table::deleteRecordsByCondition(const std::vector<IFieldValue*>& conditionValues)
	{
		std::vector<std::string> conditionValuesSQL;
		const unsigned int nConditionFieldValues = (unsigned int) conditionValues.size();
		for (unsigned int i = 0; i < nConditionFieldValues; i++)
		{
			IFieldValue& conditionFieldValue = *(conditionValues[i]);
			const IField& field = conditionFieldValue.getField();

			if (!isOwned(field))
			{
				throw std::runtime_error("Can't update records using condition values that aren't owned by this table." );
			}

			if (!conditionFieldValue.isDefault())
			{
				std::string conditionFieldValueName = field.getName();
				std::string conditionFieldValueSQLValue = getSQLValue(conditionFieldValue, true, false);
				conditionValuesSQL.push_back( conditionFieldValueName + conditionFieldValueSQLValue );
			}
		}

		if (!conditionValuesSQL.empty())
		{
			std::string conditionSQLStr = getStringList(conditionValuesSQL, " AND ");

			std::string deleteSQL = "DELETE FROM " + m_name + " " +
									"WHERE " + conditionSQLStr + ";";

			m_database.executeOperation(deleteSQL);
			return m_database.getRowsAffectedByLastChangeOperation();
		}
		else
		{
			return (RowsAffected) 0;
		}
	}

	RowsAffected Table::deleteRecordsByCondition(const std::string& condition)
	{
		std::string deleteSQL = "DELETE FROM " + m_name + " " +
								"WHERE " + condition + ";";

		m_database.executeOperation(deleteSQL);
		return m_database.getRowsAffectedByLastChangeOperation();
	}

	RowsAffected Table::deleteAllRecords()
	{
		std::string deleteSQL = "DELETE FROM " + m_name + ";";

		m_database.executeOperation(deleteSQL);
		return m_database.getRowsAffectedByLastChangeOperation();
	}

	void Table::loadFields()
	{
		std::string query = "SELECT a.attname, a.atttypid::regtype, pg_get_expr(b.adbin, b.adrelid) AS default_value, c.indisprimary "
							"FROM pg_attribute a "
							"LEFT JOIN pg_attrdef b ON (a.attrelid, a.attnum) = (b.adrelid, b.adnum) "
							"LEFT JOIN pg_index c ON a.attrelid = c.indrelid "
							"AND a.attnum = ANY(c.indkey) "
							"WHERE a.attrelid = '" + m_name + "'::regclass "
							"AND a.attnum > 0 "
							"AND NOT a.attisdropped";

		std::unique_ptr<IRecordSet> fieldsRecordSet = m_database.executeQuery(query);

		unsigned int i = 0;
		while (fieldsRecordSet->isCurrentRecordValid())
		{
			const IRecord& record = fieldsRecordSet->getCurrentRecord();
			std::string fieldName = record.getFieldValue("attname").getStringValue();
			std::string fieldTypeName = record.getFieldValue("atttypid").getStringValue();
			FieldTypes fieldType = getTypeFromPostgresTypeName(fieldTypeName);
			const auto& isPrimaryKeyValue = record.getFieldValue("indisprimary");
			bool fieldPK = false;
			if (!isPrimaryKeyValue.isNull())
			{
				fieldPK = isPrimaryKeyValue.getBooleanValue();
			}

			std::string defaultValue = "NULL";
			if (!record.getFieldValue("default_value").isNull())
			{
				defaultValue = record.getFieldValue("default_value").getStringValue();
				if (fieldType == STRING || fieldType == DATETIME)
				{
					defaultValue = defaultValue.substr(1, defaultValue.find('\'', 1) -1);
				}
			}

			auto field = std::make_unique<Field>(i, fieldName, fieldType, defaultValue, fieldPK);
			m_fields.push_back( std::move(field));

			fieldsRecordSet->nextRecord();
			i++;
		}

		if (m_fields.empty())
		{
			std::string excMessage = "Table " + m_name + " doesn't exist in database.";
			throw std::runtime_error(excMessage.c_str());
		}
	}

	bool Table::isOwned(const systelab::db::IField& field) const
	{
		const unsigned int index = field.getIndex();
		if (index < m_fields.size())
		{
			return (m_fields.at(index).get() == &field);
		}
		
		return false;
		
	}
}