#include "stdafx.h"
#include "Helpers/Helpers.h"
#include "Helpers/DefaultConnectionConfiguration.h"

#include "Connection.h"
#include "DbAdapterInterface/IDatabase.h"
#include "DbAdapterInterface/IPrimaryKeyValue.h"
#include "DbAdapterInterface/ITable.h"
#include "DbAdapterInterface/ITableRecord.h"
#include "DbAdapterInterface/ITableRecordSet.h"

namespace
{
	static const double precision = 1e-10;
	static const std::string SCHEMA_PREFIX = "public";
	static const std::string QUERY_TABLE_NAME = "QUERY_TABLE";
	static const int QUERY_TABLE_NUM_RECORDS = 100;
}

using namespace testing;
using namespace std::chrono_literals;
namespace systelab::db::postgresql::unit_test {

	/**
	 * Tests if the query operations over a table performed using the Postgres DB adapter
	 * return the data stored on that table.
	 */
	class DbQueryOperationsTest : public Test
	{
	protected:
		void SetUp() override
		{
			dropDatabase(defaultDbName);
			createDatabase(defaultDbName);

			m_db = Connection().loadDatabase(const_cast<ConnectionConfiguration&>(defaultConfiguration));
			createTable(*m_db, QUERY_TABLE_NAME, SCHEMA_PREFIX, QUERY_TABLE_NUM_RECORDS);
		}

		void TearDown() override
		{
			m_db.release();
			dropDatabase(defaultDbName);
		}

		ITable& getQueryTable() const
		{
			return m_db->getTable(getPrefixedElement(QUERY_TABLE_NAME, SCHEMA_PREFIX));
		}

		void assertRecordSet(ITableRecordSet& recordset)
		{
			while (recordset.isCurrentRecordValid())
			{
				const ITableRecord& record = recordset.getCurrentRecord();
				assertRecord(record);
				recordset.nextRecord();
			}
		}

		void assertRecord(const ITableRecord& record)
		{
			int id = record.getFieldValue("id").getIntValue() - 1; // Autoincrease starts indexes at 1, but the parameters are computed with 0 based indexes
			int fieldIntIndex = record.getFieldValue("field_int_index").getIntValue();
			int fieldIntNoIndex = record.getFieldValue("field_int_no_index").getIntValue();
			std::string fieldStrIndex = record.getFieldValue("field_str_index").getStringValue();
			std::string fieldStrNoIndex = record.getFieldValue("field_str_no_index").getStringValue();
			double fieldReal = record.getFieldValue("field_real").getDoubleValue();
			bool fieldBool = record.getFieldValue("field_bool").getBooleanValue();
			std::chrono::system_clock::time_point fieldDateTime = record.getFieldValue("field_date").getDateTimeValue();

			ASSERT_EQ(fieldIntIndex,	getFieldIntIndexValue(id));
			ASSERT_EQ(fieldIntNoIndex,	getFieldIntNoIndexValue(id));
			ASSERT_EQ(fieldStrIndex,	getFieldStringIndexValue(id));
			ASSERT_EQ(fieldStrNoIndex,	getFieldStringNoIndexValue(id));
			ASSERT_NEAR(fieldReal,		getFieldRealValue(id), precision);
			ASSERT_EQ(fieldBool,		getFieldBooleanValue(id));
			ASSERT_EQ(fieldDateTime,	getFieldDateValue(id));
		}

	private:
		std::unique_ptr<IDatabase> m_db;
	};
	

	TEST_F(DbQueryOperationsTest, testQueryAll)
	{
		std::unique_ptr<ITableRecordSet> recordset = getQueryTable().getAllRecords();
		ASSERT_EQ(QUERY_TABLE_NUM_RECORDS, recordset->getRecordsCount());
		assertRecordSet(*recordset);
	}

	TEST_F(DbQueryOperationsTest, testQueryByPrimaryKey)
	{
		ITable& table = getQueryTable();
		std::unique_ptr<IPrimaryKeyValue> primaryKeyValue = table.createPrimaryKeyValue();
		primaryKeyValue->getFieldValue("id").setIntValue(27);

		std::unique_ptr<ITableRecord> record = table.getRecordByPrimaryKey(*primaryKeyValue);
		ASSERT_THAT(record, NotNull());
		ASSERT_EQ(record->getFieldValue("id").getIntValue(), 27);
		assertRecord(*record);
	}

	TEST_F(DbQueryOperationsTest, testQueryWhenFieldIntIndexIsZero)
	{
		ITable& table = getQueryTable();
		const IField& fieldIntIndex = table.getField("field_int_index");
		std::unique_ptr<IFieldValue> fieldIntIndexValue = table.createFieldValue(fieldIntIndex, 0);

		unsigned int expectedRecords = getNumRecordsWithFieldIntIndexZero(QUERY_TABLE_NUM_RECORDS);
		std::unique_ptr<ITableRecordSet> recordset = table.filterRecordsByField(*fieldIntIndexValue);
		ASSERT_EQ(expectedRecords, recordset->getRecordsCount());

		while (recordset->isCurrentRecordValid())
		{
			const ITableRecord& record = recordset->getCurrentRecord();
			ASSERT_EQ(record.getFieldValue("field_int_index").getIntValue(), 0);
			assertRecord(record);
			recordset->nextRecord();
		}
	}

	TEST_F(DbQueryOperationsTest, testQueryWhenFieldIntNoIndexIsZero)
	{
		ITable& table = getQueryTable();
		const IField& fieldIntNoIndex = table.getField("field_int_no_index");
		std::unique_ptr<IFieldValue> fieldIntNoIndexValue = table.createFieldValue(fieldIntNoIndex, 0);

		unsigned int expectedRecords = getNumRecordsWithFieldIntNoIndexZero(QUERY_TABLE_NUM_RECORDS);
		std::unique_ptr<ITableRecordSet> recordset = table.filterRecordsByField(*fieldIntNoIndexValue);
		ASSERT_EQ(expectedRecords, recordset->getRecordsCount());

		while (recordset->isCurrentRecordValid())
		{
			const ITableRecord& record = recordset->getCurrentRecord();
			ASSERT_EQ(record.getFieldValue("field_int_no_index").getIntValue(), 0);
			assertRecord(record);
			recordset->nextRecord();
		}
	}

	TEST_F(DbQueryOperationsTest, testQueryWhenFieldStrIndexIsSTR0)
	{
		ITable& table = getQueryTable();
		const IField& fieldStrIndex = table.getField("field_str_index");
		std::unique_ptr<IFieldValue> fieldStrIndexValue = table.createFieldValue(fieldStrIndex, std::string("STR0"));

		unsigned int expectedRecords = getNumRecordsWithFieldStringIndexZero(QUERY_TABLE_NUM_RECORDS);
		std::unique_ptr<ITableRecordSet> recordset = table.filterRecordsByField(*fieldStrIndexValue);
		ASSERT_EQ(expectedRecords, recordset->getRecordsCount());

		while (recordset->isCurrentRecordValid())
		{
			const ITableRecord& record = recordset->getCurrentRecord();
			ASSERT_EQ(record.getFieldValue("field_str_index").getStringValue(), std::string("STR0"));
			assertRecord(record);
			recordset->nextRecord();
		}
	}

	TEST_F(DbQueryOperationsTest, testQueryWhenFieldStrNoIndexIsSTR0)
	{
		ITable& table = getQueryTable();
		const IField& fieldStrNoIndex = table.getField("field_str_no_index");
		std::unique_ptr<IFieldValue> fieldStrNoIndexValue = table.createFieldValue(fieldStrNoIndex, std::string("STR0"));

		int expectedRecords = getNumRecordsWithFieldStringNoIndexZero(QUERY_TABLE_NUM_RECORDS);
		std::unique_ptr<ITableRecordSet> recordset = table.filterRecordsByField(*fieldStrNoIndexValue);
		ASSERT_EQ(expectedRecords, recordset->getRecordsCount());

		while (recordset->isCurrentRecordValid())
		{
			const ITableRecord& record = recordset->getCurrentRecord();
			ASSERT_EQ(record.getFieldValue("field_str_no_index").getStringValue(), std::string("STR0"));
			assertRecord(record);
			recordset->nextRecord();
		}
	}

	TEST_F(DbQueryOperationsTest, testQueryWhenFieldRealIsZero)
	{
		ITable& table = getQueryTable();
		const IField& fieldReal = table.getField("field_real");
		std::unique_ptr<IFieldValue> fieldRealValue = table.createFieldValue(fieldReal, 0.);

		int expectedRecords = getNumRecordsWithFieldRealZero(QUERY_TABLE_NUM_RECORDS);
		std::unique_ptr<ITableRecordSet> recordset = table.filterRecordsByField(*fieldRealValue);
		ASSERT_EQ(expectedRecords, recordset->getRecordsCount());

		while (recordset->isCurrentRecordValid())
		{
			const ITableRecord& record = recordset->getCurrentRecord();
			ASSERT_EQ(record.getFieldValue("field_real").getDoubleValue(), 0.);
			assertRecord(record);
			recordset->nextRecord();
		}
	}

	TEST_F(DbQueryOperationsTest, testQueryWhenFieldBoolIsTrue)
	{
		ITable& table = getQueryTable();
		const IField& fieldBool = table.getField("field_bool");
		std::unique_ptr<IFieldValue> fieldBoolValue = table.createFieldValue(fieldBool, true);

		int expectedRecords = getNumRecordsWithFieldBoolTrue(QUERY_TABLE_NUM_RECORDS);
		std::unique_ptr<ITableRecordSet> recordset = table.filterRecordsByField(*fieldBoolValue);
		ASSERT_EQ(expectedRecords, recordset->getRecordsCount());

		while (recordset->isCurrentRecordValid())
		{
			const ITableRecord& record = recordset->getCurrentRecord();
			ASSERT_EQ(record.getFieldValue("field_bool").getBooleanValue(), true);
			assertRecord(record);
			recordset->nextRecord();
		}
	}

	TEST_F(DbQueryOperationsTest, testQueryWhenFieldDateIsBaseDate)
	{
		std::chrono::system_clock::time_point baseDate(std::chrono::sys_days{ 12d / 2 / 2024 });

		ITable& table = getQueryTable();
		const IField& fieldDate = table.getField("field_date");
		std::unique_ptr<IFieldValue> fieldDateValue = table.createFieldValue(fieldDate, baseDate);

		int expectedRecords = getNumRecordsWithFieldDateIsBaseDate(QUERY_TABLE_NUM_RECORDS);
		std::unique_ptr<ITableRecordSet> recordset = table.filterRecordsByField(*fieldDateValue);
		ASSERT_EQ(expectedRecords, recordset->getRecordsCount());

		while (recordset->isCurrentRecordValid())
		{
			const ITableRecord& record = recordset->getCurrentRecord();
			ASSERT_EQ(record.getFieldValue("field_date").getDateTimeValue(), baseDate);
			assertRecord(record);
			recordset->nextRecord();
		}
	}
}