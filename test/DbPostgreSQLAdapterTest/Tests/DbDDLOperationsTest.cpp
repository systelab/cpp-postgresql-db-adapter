#include "stdafx.h"

#include "Connection.h"
#include "ConnectionConfiguration.h"
#include "DbAdapterInterface/IDatabase.h"
#include "DbAdapterInterface/IRecordSet.h"
#include "Helpers/Helpers.h"
#include "Helpers/DefaultConnectionConfiguration.h"

namespace
{
	static const std::string SCHEMA_PREFIX = "public";
	static const std::string MAIN_TABLE = "MAIN_TABLE";
	static const std::string DUMMY_TABLE = "DUMMY_TABLE";

	bool checkIfTableExists(std::string table, std::unique_ptr<systelab::db::IDatabase>& database)
	{
		const std::string tableQuery = "SELECT tableName FROM pg_catalog.pg_tables WHERE tableName='" + table + "';";

		// Check there is no table at the database with the given name:
		std::unique_ptr<systelab::db::IRecordSet> recordSet = database->executeQuery(tableQuery);
		return recordSet->getRecordsCount() == 1;
	}

	unsigned int getNumberOfIndexes(std::string table, std::unique_ptr<systelab::db::IDatabase>& database)
	{
		const std::string tableQuery = "SELECT tableName FROM pg_catalog.pg_indexes WHERE tableName='" + table + "';";

		// Check there is no table at the database with the given name:
		std::unique_ptr<systelab::db::IRecordSet> recordSet = database->executeQuery(tableQuery);
		return recordSet->getRecordsCount();
	}
}

using namespace testing;
namespace systelab::db::postgresql::unit_test {
		
	class DbDDLOperationsTest: public Test
	{
	public:
		void SetUp() override
		{
			dropDatabase(defaultDbName);
			createDatabase(defaultDbName);
			
			m_db = Connection().loadDatabase(const_cast<ConnectionConfiguration&>(defaultConfiguration));
			createTable(*m_db, MAIN_TABLE, SCHEMA_PREFIX, 100);
		}

	public:
		std::unique_ptr<IDatabase> m_db;
	};

	// Create Table
	TEST_F(DbDDLOperationsTest, testDDLCreateTable)
	{
		ASSERT_FALSE(checkIfTableExists(DUMMY_TABLE, m_db));

		// Creates an empty table and checks if the table exists within the DB afterwards.
		m_db->executeOperation("CREATE TABLE " + getPrefixedElement(DUMMY_TABLE, SCHEMA_PREFIX) + " (ID INT PRIMARY KEY NOT NULL, FIELD_INT_INDEX INT, FIELD_INT_NO_INDEX INT, FIELD_STR_INDEX VARCHAR(255), FIELD_STR_NO_INDEX VARCHAR(255), FIELD_DATE TIMESTAMP WITH TIME ZONE DEFAULT '2024-02-12T00:00:00.000')");
		
		ASSERT_TRUE(checkIfTableExists(DUMMY_TABLE, m_db));
	}

	TEST_F(DbDDLOperationsTest, testDDLCreateInvalidTable)
	{
		ASSERT_TRUE(checkIfTableExists(MAIN_TABLE, m_db));

		// Attempts to create a table tha already exists in the database.
		ASSERT_THROW(m_db->executeOperation("CREATE TABLE " + getPrefixedElement(MAIN_TABLE, SCHEMA_PREFIX) + " (ID INT PRIMARY KEY NOT NULL, FIELD_INT_INDEX INT, FIELD_INT_NO_INDEX INT, FIELD_STR_INDEX VARCHAR(255), FIELD_STR_NO_INDEX VARCHAR(255))"), std::exception);
		
		ASSERT_TRUE(checkIfTableExists(MAIN_TABLE, m_db));
	}

	TEST_F(DbDDLOperationsTest, testDDLDropTable)
	{
		// Creates an empty table and checks if the table exists within the DB afterwards.
		m_db->executeOperation("CREATE TABLE " + getPrefixedElement(DUMMY_TABLE, SCHEMA_PREFIX) + " (ID INT PRIMARY KEY NOT NULL, FIELD_INT_INDEX INT, FIELD_INT_NO_INDEX INT, FIELD_STR_INDEX VARCHAR(255), FIELD_STR_NO_INDEX VARCHAR(255), FIELD_DATE TIMESTAMP WITH TIME ZONE DEFAULT '2024-02-12T01:00:00.000')");
		ASSERT_TRUE(checkIfTableExists(DUMMY_TABLE, m_db));

		m_db->executeOperation("DROP TABLE " + getPrefixedElement(DUMMY_TABLE, SCHEMA_PREFIX));
		ASSERT_FALSE(checkIfTableExists(DUMMY_TABLE, m_db));
	}

	TEST_F(DbDDLOperationsTest, testDDLDropInvalidTable)
	{
		ASSERT_FALSE(checkIfTableExists(DUMMY_TABLE, m_db));

		// Attempts to drop a non-existing table from the database.
		ASSERT_THROW(m_db->executeOperation("DROP TABLE " + DUMMY_TABLE), std::exception);

		ASSERT_FALSE(checkIfTableExists(DUMMY_TABLE, m_db));
	}

	TEST_F(DbDDLOperationsTest, testDDLCreateValidIndex)
	{
		const std::string indexName = "INT_INDEX_2";
		const std::string indexFieldName = "FIELD_INT_INDEX";

		// Creates an empty table and checks if the table exists within the DB afterwards.
		m_db->executeOperation("CREATE TABLE " + getPrefixedElement(DUMMY_TABLE, SCHEMA_PREFIX) + " (ID INT PRIMARY KEY NOT NULL, " + indexFieldName + " INT, FIELD_INT_NO_INDEX INT, FIELD_STR_INDEX VARCHAR(255), FIELD_STR_NO_INDEX VARCHAR(255), FIELD_DATE TIMESTAMP WITH TIME ZONE DEFAULT '2024-02-12T01:00:00.000')");

		// Check we have 1 indexes at the table before creating the index (the primary key generates an index):
		ASSERT_EQ(1, getNumberOfIndexes(DUMMY_TABLE, m_db));

		// Create the index, whose name has to be unique in the whole database.
		m_db->executeOperation("CREATE INDEX " + indexName + " ON " + getPrefixedElement(DUMMY_TABLE, SCHEMA_PREFIX) + "(" + indexFieldName + ")");

		// Check we have 2 indexes at the table after creating the index:
		ASSERT_EQ(2, getNumberOfIndexes(DUMMY_TABLE, m_db));
	}

	TEST_F(DbDDLOperationsTest, testDDLCreateInvalidIndex)
	{
		const std::string indexName = "\"INT_INDEX_" + MAIN_TABLE +'"';
		const std::string indexFieldName = "FIELD_INT_INDEX";

		// Creates an empty table and checks if the table exists within the DB afterwards.
		m_db->executeOperation("CREATE TABLE " + getPrefixedElement(DUMMY_TABLE, SCHEMA_PREFIX) + " (ID INT PRIMARY KEY NOT NULL, " + indexFieldName + " INT, FIELD_INT_NO_INDEX INT, FIELD_STR_INDEX VARCHAR(255), FIELD_STR_NO_INDEX VARCHAR(255), FIELD_DATE TIMESTAMP WITH TIME ZONE DEFAULT '2024-02-12T01:00:00.000')");

		// Check we have 1 indexes at the table before creating the index (the primary key generates an index):
		ASSERT_EQ(1, getNumberOfIndexes(DUMMY_TABLE, m_db));

		// Create an index already existent in current DB (in MAIN_TABLE)
		ASSERT_THROW(m_db->executeOperation("CREATE INDEX " + indexName + " ON " + getPrefixedElement(DUMMY_TABLE, SCHEMA_PREFIX) + "(" + indexFieldName + ")"), std::exception); // Create the index, whose name has to be unique in the whole database.

		// Check we still have 1 index after attempting to create the index:
		ASSERT_EQ(1, getNumberOfIndexes(DUMMY_TABLE, m_db));
	}

	// Drop index
	TEST_F(DbDDLOperationsTest, testDDLDropValidIndex)
	{
		const std::string indexName = getPrefixedElement("INT_INDEX_" + MAIN_TABLE, SCHEMA_PREFIX);

		// Check that we have 3 indexes (primary key + 2 created indexes) at MAIN_TABLE.
		ASSERT_EQ(3, getNumberOfIndexes(MAIN_TABLE, m_db));

		// Drop an index from the table
		m_db->executeOperation("DROP INDEX " + indexName);

		ASSERT_EQ(2, getNumberOfIndexes(MAIN_TABLE, m_db));
	}

	TEST_F(DbDDLOperationsTest, testDDLDropInvalidIndex)
	{
		std::string indexName = "INT_INDEX24";

		// Check that we have 3 indexes (primary key + 2 created indexes) at MAIN_TABLE.
		ASSERT_EQ(3, getNumberOfIndexes(MAIN_TABLE, m_db));

		// Drop an index from the table
		ASSERT_THROW(m_db->executeOperation("DROP INDEX " + indexName), std::exception);

		ASSERT_EQ(3, getNumberOfIndexes(MAIN_TABLE, m_db));
	}
}