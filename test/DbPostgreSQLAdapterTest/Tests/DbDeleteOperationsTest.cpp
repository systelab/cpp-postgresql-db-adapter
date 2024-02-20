#include "stdafx.h"

#include "Connection.h"
#include "ConnectionConfiguration.h"
#include "DbAdapterInterface/IDatabase.h"
#include "DbAdapterInterface/IFieldValue.h"
#include "DbAdapterInterface/IPrimaryKeyValue.h"
#include "DbAdapterInterface/ITable.h"
#include "Helpers/Helpers.h"
#include "Helpers/DefaultConnectionConfiguration.h"

namespace {
	static const std::string SCHEMA_PREFIX = "public";
	static const std::string DELETE_TABLE_NAME = "DELETE_TABLE";
	static const int DELETE_TABLE_NUM_RECORDS = 50;
}

using namespace testing;
namespace systelab::db::postgresql::unit_test {

	/**
	 * Tests if the delete operations over a table performed using the Postgres DB adapter
	 * work properly.
	 */
	class DbDeleteOperationsTest: public Test
	{
	public:
		void SetUp() override
		{
			dropDatabase(defaultDbName);
			createDatabase(defaultDbName);

			m_db = Connection().loadDatabase(const_cast<ConnectionConfiguration&>(defaultConfiguration));
			createTable(*m_db, DELETE_TABLE_NAME, SCHEMA_PREFIX, DELETE_TABLE_NUM_RECORDS);
		}

		ITable& getDeleteTable() const
		{
			return m_db->getTable(getPrefixedElement(DELETE_TABLE_NAME, SCHEMA_PREFIX));
		}

	private:
		std::unique_ptr<IDatabase> m_db;
	};


	TEST_F(DbDeleteOperationsTest, testDeleteSingleRecord)
	{
		ITable& table = getDeleteTable();
		std::unique_ptr<IPrimaryKeyValue> primaryKeyValue = table.createPrimaryKeyValue();
		primaryKeyValue->getFieldValue("id").setIntValue(14);

		// Delete the single record
		RowsAffected nRows = table.deleteRecord(*primaryKeyValue);
		ASSERT_EQ(nRows, 1);

		// Check that the record has been deleted successfully
		std::unique_ptr<ITableRecord> nullRecord = table.getRecordByPrimaryKey(*primaryKeyValue);
		ASSERT_THAT(nullRecord, IsNull());
	}

	TEST_F(DbDeleteOperationsTest, testDeleteNonExistingRecordAffectsZeroRows)
	{
		ITable& table = getDeleteTable();
		std::unique_ptr<ITableRecord> nonExistingRecord = table.createRecord();
		nonExistingRecord->getFieldValue("id").setIntValue(-1);

		RowsAffected nRows = table.deleteRecord(*nonExistingRecord);
		ASSERT_EQ(nRows, 0);
	}

	TEST_F(DbDeleteOperationsTest, testDeleteMultipleRecordsByCondition)
	{
		ITable& table = getDeleteTable();
		std::unique_ptr<IFieldValue> strIndexValue = table.createFieldValue(table.getField("field_str_index"), std::string("STR0"));

		std::vector<IFieldValue*> conditionValues;
		conditionValues.push_back(&(*strIndexValue));

		int expectedAffectedRows = (int) ceil (DELETE_TABLE_NUM_RECORDS / 9.);
		RowsAffected nRows = table.deleteRecordsByCondition(conditionValues);
		ASSERT_EQ(nRows, expectedAffectedRows);

		std::unique_ptr<ITableRecordSet> recordset = table.filterRecordsByFields(conditionValues);
		ASSERT_EQ(recordset->getRecordsCount(), 0);
	}
}