#include "stdafx.h"

#include "Connection.h"
#include "DbAdapterInterface/IDatabase.h"
#include "DbAdapterInterface/ITable.h"
#include "DbAdapterInterface/ITableRecord.h"
#include "DbAdapterInterface/ITransaction.h"
#include "Helpers/Helpers.h"
#include "Helpers/DefaultConnectionConfiguration.h"

namespace
{
	static const std::string SCHEMA_PREFIX = "public";
	static const std::string MAIN_TABLE = "MAIN_TABLE";
	static const std::string DUMMY_TABLE = "DUMMY_TABLE";
}

using namespace testing;
namespace systelab::db::postgresql::unit_test {


	class DbTransactionsTest: public Test
	{
	protected:
		void SetUp()
		{
			dropDatabase(defaultDbName);
			createDatabase(defaultDbName);

			m_db = Connection().loadDatabase(const_cast<ConnectionConfiguration&>(defaultConfiguration));
			createTable(*m_db, DUMMY_TABLE, SCHEMA_PREFIX, 0);
		}

		void TearDown() override
		{
			m_db.reset();
			dropDatabase(defaultDbName);
		}

		std::unique_ptr<IDatabase> m_db;
	};
	
	// INTENDED USE 7: Transactions.

	TEST_F(DbTransactionsTest, testSQLOperationsInsertFieldsWithTransactionCommit)
	{
		ITable& table = m_db->getTable(getPrefixedElement(DUMMY_TABLE, SCHEMA_PREFIX));
		
		std::unique_ptr<ITransaction> transaction = m_db->startTransaction();
		for (unsigned int i = 0; i<4; i++)
		{
			// Create the record to insert
			std::unique_ptr<ITableRecord> record = table.createRecord();
			record->getFieldValue("id").setIntValue(i+1);
			record->getFieldValue("field_int_index").setIntValue(2552);
			record->getFieldValue("field_str_index").setStringValue("hola");

			// Add the record to the table:
			table.insertRecord(*record);
		}

		transaction->commit();
		transaction.reset();

		// Check table contains inserted fields;
		std::unique_ptr<ITableRecordSet> recordSet = table.getAllRecords();
		ASSERT_EQ(recordSet->getRecordsCount(), 4);
	}	

	TEST_F(DbTransactionsTest, testSQLOperationsInsertFieldsWithTransactionRollback)
	{
		ITable& table = m_db->getTable(getPrefixedElement(DUMMY_TABLE, SCHEMA_PREFIX));

		std::unique_ptr<ITransaction> transaction = m_db->startTransaction();
		for (unsigned int i = 0; i<4; i++)
		{
			// Create the record to insert
			std::unique_ptr<ITableRecord> record = table.createRecord();
			record->getFieldValue("id").setIntValue(i+1);
			record->getFieldValue("field_int_index").setIntValue(2552);
			record->getFieldValue("field_str_index").setStringValue("hola");

			// Add the record to the table:
			table.insertRecord(*record);
		}

		transaction->rollback();
		transaction.reset();

		// Check table contains inserted fields;
		std::unique_ptr<ITableRecordSet> recordSet = table.getAllRecords();
		ASSERT_EQ(recordSet->getRecordsCount(), 0);
	}

	// Delete with transaction commit
	TEST_F(DbTransactionsTest, testSQLOperationsDeleteRecordWithTransactionCommit)
	{
		createTable(*m_db, MAIN_TABLE, SCHEMA_PREFIX, 25);

		ITable& table = m_db->getTable(getPrefixedElement(MAIN_TABLE, SCHEMA_PREFIX));
		
		std::unique_ptr<ITransaction> transaction = m_db->startTransaction();

		// Now check there is no record on the table with ID = 3;
		std::unique_ptr<ITableRecordSet> recordSet = table.filterRecordsByCondition("id = 3");
		
		std::unique_ptr<ITableRecord> record = table.createRecord();
		record->getFieldValue("id").setIntValue(3);

		std::vector<IFieldValue*> fieldValues;
		fieldValues.push_back(&record->getFieldValue("id"));

		int result = table.deleteRecordsByCondition(fieldValues);

		transaction->commit();
		transaction.reset();
		
		// Now check there is no record on the table with ID = 3;
		recordSet = table.filterRecordsByCondition("id = 3");
		
		ASSERT_EQ(recordSet->getRecordsCount(), 0);
	}

	// Delete with transaction rollback
	TEST_F(DbTransactionsTest, testSQLOperationsDeleteRecordWithTransactionRollback)
	{
		IDatabase& db = *m_db;

		createTable(db, MAIN_TABLE, SCHEMA_PREFIX, 25);

		ITable& table = m_db->getTable(getPrefixedElement(MAIN_TABLE, SCHEMA_PREFIX));
		
		std::unique_ptr<ITransaction> transaction = db.startTransaction();

		// Now check there is no record on the table with ID = 3;
		std::unique_ptr<ITableRecordSet> recordSet = table.filterRecordsByCondition("id = 3");
		
		std::unique_ptr<ITableRecord> record = table.createRecord();
		record->getFieldValue("id").setIntValue(3);

		std::vector<IFieldValue*> fieldValues;
		fieldValues.push_back(&record->getFieldValue("id"));

		int result = table.deleteRecordsByCondition(fieldValues);

		transaction->rollback();
		transaction.reset();

		// Now check there is no record on the table with ID = 3;
		recordSet = table.filterRecordsByCondition("id = 3");
		
		ASSERT_EQ(recordSet->getRecordsCount(), 1);
	}
}