#include "stdafx.h"
#include "Helpers/Helpers.h"
#include "Helpers/DefaultConnectionConfiguration.h"

#include "Connection.h"
#include "ConnectionConfiguration.h"
#include "DbAdapterInterface/IDatabase.h"
#include "DbAdapterInterface/ITable.h"
#include "DbAdapterInterface/ITableRecord.h"

namespace {
	static const double precision = 1e-10;
	static const std::string SCHEMA_PREFIX = "public";
}

using namespace testing;
using namespace std::chrono_literals;
namespace systelab::db::postgresql::unit_test {

	static const std::string INSERT_TABLE_NAME = "INSERT_TABLE";
	static const int INSERT_TABLE_NUM_RECORDS = 10;
	/**
	 * Tests if the insert operations over a table performed using the Postgres DB adapter
	 * work properly.
	 */
	class DbInsertOperationsTest: public Test
	{
	protected:
		void SetUp()
		{
			dropDatabase(defaultDbName);
			createDatabase(defaultDbName);

			m_db = Connection().loadDatabase(const_cast<ConnectionConfiguration&>(defaultConfiguration));
			createTable(*m_db, INSERT_TABLE_NAME, SCHEMA_PREFIX, INSERT_TABLE_NUM_RECORDS);
		}

		void TearDown() override
		{
			dropDatabase(defaultDbName);
		}

		ITable& getInsertTable() const
		{
			return m_db->getTable(getPrefixedElement(INSERT_TABLE_NAME, SCHEMA_PREFIX));
		}

	private:
		std::unique_ptr<IDatabase> m_db;
	};

	TEST_F(DbInsertOperationsTest, testNewRecordsAreInsertedSuccessfully)
	{
		// Insert new records into table
		ITable& table = getInsertTable();
		for (unsigned int i = INSERT_TABLE_NUM_RECORDS + 1; i < INSERT_TABLE_NUM_RECORDS + 6; i++ )
		{
			std::unique_ptr<ITableRecord> record = table.createRecord();

			record->getFieldValue("id").setIntValue(i);
			record->getFieldValue("field_int_index").setIntValue(2552 + i);
			record->getFieldValue("field_int_no_index").setIntValue(140 + i);
			record->getFieldValue("field_str_index").setStringValue(std::string("STR") + std::to_string((long long) i));
			record->getFieldValue("field_str_no_index").setStringValue(std::string("STR") + std::to_string((long long) (i*2)));
			record->getFieldValue("field_real").setDoubleValue(1.2345 + i);
			record->getFieldValue("field_bool").setBooleanValue((i%2) == 0);
			record->getFieldValue("field_date").setDateTimeValue(getFieldDateValue(i));

			RowsAffected nRows = table.insertRecord(*record.get());
			ASSERT_EQ(nRows, 1);
		}

		// Check new records have been stored successfully
		std::string conditionSQL = "id >= " + std::to_string(INSERT_TABLE_NUM_RECORDS + 1);
		std::unique_ptr<ITableRecordSet> recordset = table.filterRecordsByCondition(conditionSQL);
		ASSERT_EQ(recordset->getRecordsCount(), 5);

		while(recordset->isCurrentRecordValid())
		{
			const ITableRecord& record = recordset->getCurrentRecord();

			int id = record.getFieldValue("id").getIntValue();
			int expectedIntIndex = 2552 + id;
			int expectedIntNoIndex = 140 + id;
			std::string expectedStrIndex = "STR" + std::to_string((long long) id);
			std::string expectedStrNoIndex = "STR" + std::to_string((long long) (id*2));
			double expetedReal = 1.2345 + id;
			bool expectedBool = ((id%2) == 0);
			std::chrono::system_clock::time_point expectedDate(getFieldDateValue(id));

			ASSERT_EQ(record.getFieldValue("field_int_index").getIntValue(),		expectedIntIndex);
			ASSERT_EQ(record.getFieldValue("field_int_no_index").getIntValue(),		expectedIntNoIndex);
			ASSERT_EQ(record.getFieldValue("field_str_index").getStringValue(),		expectedStrIndex);
			ASSERT_EQ(record.getFieldValue("field_str_no_index").getStringValue(),	expectedStrNoIndex);
			ASSERT_NEAR(record.getFieldValue("field_real").getDoubleValue(),		expetedReal, precision);
			ASSERT_EQ(record.getFieldValue("field_bool").getBooleanValue(),			expectedBool);
			ASSERT_EQ(record.getFieldValue("field_date").getDateTimeValue(),		expectedDate);

			recordset->nextRecord();
		}
	}

	TEST_F(DbInsertOperationsTest, testInsertRecordThrowsAnExceptionIfRecordAlreadyExists)
	{
		// Create the record to insert
		ITable& table = getInsertTable();
		std::unique_ptr<ITableRecord> record = table.createRecord();
		record->getFieldValue("id").setIntValue(25);
		record->getFieldValue("field_int_index").setIntValue(1);
		record->getFieldValue("field_int_no_index").setIntValue(1);
		record->getFieldValue("field_str_index").setStringValue("STR");
		record->getFieldValue("field_str_no_index").setStringValue("STR");
		record->getFieldValue("field_real").setDoubleValue(1.0);
		record->getFieldValue("field_bool").setBooleanValue(true);
		record->getFieldValue("field_date").setDateTimeValue(std::chrono::system_clock::time_point{});

		// Insert the record into table
		RowsAffected nRows = table.insertRecord(*record.get());
		ASSERT_EQ(nRows, 1);

		// Try to insert again the same record (without success)
		ASSERT_THROW(table.insertRecord(*record.get()), std::exception);
	}

	TEST_F(DbInsertOperationsTest, testInsertRecordFillsRecordWithGeneratedIdentifierAndDefaultValues)
	{
		// Create the record to insert
		ITable& table = getInsertTable();
		std::unique_ptr<ITableRecord> record = table.createRecord();
		record->getFieldValue("field_int_index").setIntValue(1);
		record->getFieldValue("field_str_index").setStringValue("STR");

		RowsAffected nRows = table.insertRecord(*record.get());
		ASSERT_EQ(1, nRows);

		// Check the identifier of the inserted record
		ASSERT_EQ(INSERT_TABLE_NUM_RECORDS + 1, record->getFieldValue("id").getIntValue());

		// Check that values of the inserted record are the default values
		ASSERT_EQ(2, record->getFieldValue("field_int_no_index").getIntValue());
		ASSERT_EQ("FIELD_STR_NO_INDEX", record->getFieldValue("field_str_no_index").getStringValue());
		ASSERT_NEAR(3.3, record->getFieldValue("field_real").getDoubleValue(), precision);
		ASSERT_EQ(false, record->getFieldValue("field_bool").getBooleanValue());

		const std::chrono::system_clock::time_point expectedDateTime{ std::chrono::sys_days{2d / 1 / 2016} + 2h + 4min + 5s };
		ASSERT_EQ(expectedDateTime, record->getFieldValue("field_date").getDateTimeValue());
	}


	/**
	* Tests if conncurrent insert operations over different tables performed using the Postgres DB adapter
	* work properly.
	*/
	static const std::string INSERT_TABLE1_NAME = "INSERT_TABLE1";
	static const std::string INSERT_TABLE2_NAME = "INSERT_TABLE2";
	static const int INSERT_TABLE1_NUM_RECORDS = 0;
	static const int INSERT_TABLE2_NUM_RECORDS = 100;

	class DbInsertOperationsConcurrentTest: public Test
	{
	public:
		void SetUp()
		{
			dropDatabase(defaultDbName);
			createDatabase(defaultDbName);

			m_db = Connection().loadDatabase(const_cast<ConnectionConfiguration&>(defaultConfiguration));
			createTable(*m_db, INSERT_TABLE1_NAME, SCHEMA_PREFIX, INSERT_TABLE1_NUM_RECORDS);
			createTable(*m_db, INSERT_TABLE2_NAME, SCHEMA_PREFIX, INSERT_TABLE2_NUM_RECORDS);
		}

		void TearDown() override
		{
			m_db.release();
			dropDatabase(defaultDbName);
		}

		ITable& getInsertTable1() const
		{
			return m_db->getTable(getPrefixedElement(INSERT_TABLE1_NAME, SCHEMA_PREFIX));
		}

		ITable& getInsertTable2() const
		{
			return m_db->getTable(getPrefixedElement(INSERT_TABLE2_NAME, SCHEMA_PREFIX));
		}

	private:
		std::unique_ptr<IDatabase> m_db;
	};

	TEST_F(DbInsertOperationsConcurrentTest, testConcurrentInsertsOnDifferentTablesDoNotCollide)
	{
		ITable& table1 = getInsertTable1();
		ITable& table2 = getInsertTable2();
		unsigned int nInserts = 250;

		std::thread insertsOnTable1Thread(
			[&table1, nInserts]()
			{
				for (unsigned int i = 0; i < nInserts; i++)
				{
					std::unique_ptr<ITableRecord> newRecord = table1.createRecord();
					newRecord->getFieldValue("field_int_index").setIntValue(101 + i);
					newRecord->getFieldValue("field_str_index").setStringValue("NEW-RECORD-TABLE1-" + std::to_string(i));

					RowsAffected nRows = table1.insertRecord(*newRecord.get());

					unsigned int expectedRecordId = INSERT_TABLE1_NUM_RECORDS + 1 + i;
					EXPECT_EQ(expectedRecordId, newRecord->getFieldValue("id").getIntValue()) << " for insertion #" << i;
				}
			}
		);

		std::thread insertsOnTable2Thread(
			[&table2, nInserts]()
			{
				for (unsigned int i = 0; i < nInserts; i++)
				{
					std::unique_ptr<ITableRecord> newRecord = table2.createRecord();
					newRecord->getFieldValue("field_int_index").setIntValue(202 + i);
					newRecord->getFieldValue("field_str_index").setStringValue("NEW-RECORD-TABLE2-" + std::to_string(i));

					RowsAffected nRows = table2.insertRecord(*newRecord.get());

					unsigned int expectedRecordId = INSERT_TABLE2_NUM_RECORDS + 1 + i;
					EXPECT_EQ(expectedRecordId, newRecord->getFieldValue("id").getIntValue()) << " for insertion #" << i;
				}
			}
		);

		insertsOnTable1Thread.join();
		insertsOnTable2Thread.join();
	}
}
