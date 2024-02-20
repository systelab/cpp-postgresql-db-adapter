#include "stdafx.h"

#include "Connection.h"
#include "DbAdapterInterface/IDatabase.h"
#include "DbAdapterInterface/IPrimaryKeyValue.h"
#include "DbAdapterInterface/ITable.h"
#include "Helpers/Helpers.h"
#include "Helpers/DefaultConnectionConfiguration.h"

namespace
{
	static const double precision = 1e-4;

	static const std::string SCHEMA_PREFIX = "public";
	static const std::string UPDATE_TABLE_NAME = "UPDATE_TABLE";
	static const int UPDATE_TABLE_NUM_RECORDS = 25;
}

using namespace testing;
using namespace std::chrono_literals;
namespace systelab::db::postgresql::unit_test {

	/**
	 * Tests if the update operations over a table performed using the Postgress DB adapter
	 * work properly.
	 */
	class DbUpdateOperationsTest: public Test
	{
	protected:
		void SetUp() override
		{
			dropDatabase(defaultDbName);
			createDatabase(defaultDbName);

			m_db = Connection().loadDatabase(const_cast<ConnectionConfiguration&>(defaultConfiguration));
			createTable(*m_db, UPDATE_TABLE_NAME, SCHEMA_PREFIX, UPDATE_TABLE_NUM_RECORDS);
		}

		void TearDown() override
		{
			m_db.reset();
			dropDatabase(defaultDbName);
		}

		ITable& getUpdateTable() const
		{
			return m_db->getTable(getPrefixedElement(UPDATE_TABLE_NAME, SCHEMA_PREFIX));
		}

	private:
		std::unique_ptr<IDatabase> m_db;
	};


	TEST_F(DbUpdateOperationsTest, testUpdateSingleRecord)
	{
		// Prepare the new values for the single record
		ITable& table = getUpdateTable();
		std::unique_ptr<IPrimaryKeyValue> primaryKeyValue = table.createPrimaryKeyValue();
		primaryKeyValue->getFieldValue("id").setIntValue(4);

		const int expectedIntIndex = 1234;
		const int expectedIntNoIndex = 4321;
		const std::string expectedStrIndex = "UPDATED_STR";
		const std::string expectedStrNoIndex = "STR_UPDATED";
		const double expectedReal = 6789.1234;
		const bool expectedBool = false;
		const auto expectedDateTime = std::chrono::system_clock::time_point(std::chrono::sys_days{ 6d / 9 / 2015 });

		std::unique_ptr<ITableRecord> record = table.getRecordByPrimaryKey(*primaryKeyValue);
		record->getFieldValue("field_int_index").setIntValue(expectedIntIndex);
		record->getFieldValue("field_int_no_index").setIntValue(expectedIntNoIndex);
		record->getFieldValue("field_str_index").setStringValue(expectedStrIndex);
		record->getFieldValue("field_str_no_index").setStringValue(expectedStrNoIndex);
		record->getFieldValue("field_real").setDoubleValue(expectedReal);
		record->getFieldValue("field_bool").setBooleanValue(expectedBool);
		record->getFieldValue("field_date").setDateTimeValue(expectedDateTime);

		// Update the single record
		RowsAffected nRows = table.updateRecord(*record);
		ASSERT_EQ(nRows, 1);

		// Check that the record has been updated successfully
		std::unique_ptr<ITableRecord> updatedRecord = table.getRecordByPrimaryKey(*primaryKeyValue);
		ASSERT_EQ(updatedRecord->getFieldValue("field_int_index").getIntValue(),		expectedIntIndex);
		ASSERT_EQ(updatedRecord->getFieldValue("field_int_no_index").getIntValue(),		expectedIntNoIndex);
		ASSERT_EQ(updatedRecord->getFieldValue("field_str_index").getStringValue(),		expectedStrIndex);
		ASSERT_EQ(updatedRecord->getFieldValue("field_str_no_index").getStringValue(),	expectedStrNoIndex);
		ASSERT_NEAR(updatedRecord->getFieldValue("field_real").getDoubleValue(),			expectedReal, precision);
		ASSERT_EQ(updatedRecord->getFieldValue("field_bool").getBooleanValue(),			expectedBool);
		ASSERT_EQ(updatedRecord->getFieldValue("field_date").getDateTimeValue(),		expectedDateTime);
	}

	TEST_F(DbUpdateOperationsTest, testUpdateNonExistingRecordAffectsZeroRows)
	{
		ITable& table = getUpdateTable();
		std::unique_ptr<ITableRecord> nonExistingRecord = table.createRecord();
		nonExistingRecord->getFieldValue("id").setIntValue(-1);
		nonExistingRecord->getFieldValue("field_int_index").setIntValue(123);

		RowsAffected nRows = table.updateRecord(*nonExistingRecord);
		ASSERT_EQ(nRows, 0);
	}

	TEST_F(DbUpdateOperationsTest, testUpdateMultipleRecords)
	{
		// Prepare the new values and the condition value
		ITable& table = getUpdateTable();

		const int expectedIntIndex = 0;
		const int expectedIntNoIndex = 123;
		const std::string expectedStrIndex = "NEW_VALUE";
		const std::string expectedStrNoIndex = "NEW_VALUE2";
		const double expectedReal = 777.888;
		const bool expectedBool = false;
		const auto expectedDateTime = std::chrono::system_clock::time_point(std::chrono::sys_days{ 5d / 4 / 2012 });

		std::unique_ptr<ITableRecord> auxRecord = table.createRecord();
		auxRecord->getFieldValue("field_int_index").setIntValue(expectedIntIndex);
		auxRecord->getFieldValue("field_int_no_index").setIntValue(expectedIntNoIndex);
		auxRecord->getFieldValue("field_str_index").setStringValue(expectedStrIndex);
		auxRecord->getFieldValue("field_str_no_index").setStringValue(expectedStrNoIndex);
		auxRecord->getFieldValue("field_real").setDoubleValue(expectedReal);
		auxRecord->getFieldValue("field_bool").setBooleanValue(expectedBool);
		auxRecord->getFieldValue("field_date").setDateTimeValue(expectedDateTime);

		std::vector<IFieldValue*> newValues;
		newValues.push_back( &(auxRecord->getFieldValue("field_int_index")) );
		newValues.push_back(&(auxRecord->getFieldValue("field_int_no_index")));
		newValues.push_back( &(auxRecord->getFieldValue("field_str_index")) );
		newValues.push_back( &(auxRecord->getFieldValue("field_str_no_index")) );
		newValues.push_back( &(auxRecord->getFieldValue("field_real")) );
		newValues.push_back( &(auxRecord->getFieldValue("field_bool")) );
		newValues.push_back( &(auxRecord->getFieldValue("field_date")) );

		std::vector<IFieldValue*> conditionValues;
		conditionValues.push_back( &(auxRecord->getFieldValue("field_int_index")) );

		// Update multiple records
		int expectedAffectedRows = (int) ceil (UPDATE_TABLE_NUM_RECORDS / 7.);
		RowsAffected nRows = table.updateRecordsByCondition(newValues, conditionValues);
		ASSERT_EQ(nRows, expectedAffectedRows);

		// Check records have been updated successfully
		std::unique_ptr<ITableRecordSet> updatedRecordset = table.filterRecordsByFields(conditionValues);
		while (updatedRecordset->isCurrentRecordValid())
		{
			const ITableRecord& updatedRecord = updatedRecordset->getCurrentRecord();
			ASSERT_EQ(updatedRecord.getFieldValue("field_int_index").getIntValue(),			expectedIntIndex);
			ASSERT_EQ(updatedRecord.getFieldValue("field_int_no_index").getIntValue(),		expectedIntNoIndex);
			ASSERT_EQ(updatedRecord.getFieldValue("field_str_index").getStringValue(),		expectedStrIndex);
			ASSERT_EQ(updatedRecord.getFieldValue("field_str_no_index").getStringValue(),	expectedStrNoIndex);
			ASSERT_NEAR(updatedRecord.getFieldValue("field_real").getDoubleValue(),			expectedReal, precision);
			ASSERT_EQ(updatedRecord.getFieldValue("field_bool").getBooleanValue(),			expectedBool);
			ASSERT_EQ(updatedRecord.getFieldValue("field_date").getDateTimeValue(),			expectedDateTime);
			updatedRecordset->nextRecord();
		}
	}
}
