#include "stdafx.h"

#include "Connection.h"
#include "ConnectionConfiguration.h"
#include "DbAdapterInterface/IDatabase.h"
#include "DbAdapterInterface/IRecordSet.h"
#include "Helpers/Helpers.h"
#include "Helpers/DefaultConnectionConfiguration.h"

namespace {
	static const std::string DDL_DB_NAME = "dummyDB";
	static const std::string DDL_DB_HOST = "localhost";
	static const std::string DDL_DB_USER = "testUser";
	static const std::string DDL_DB_PASSWORD = "testPassword";
	static const std::string DDL_DB_PORT = "5432";

	static const std::string SCHEMA_PREFIX = "public";
	static const std::string TABLE_1 = "TABLE_1";
	static const std::string TABLE_FOREIGN_KEYS = "TABLE_FOREIGN_KEYS";
}

using namespace testing;
namespace systelab::db::postgresql::unit_test {

	class DbForeignKeysTest: public Test
	{
	protected:
		void SetUp()
		{
			dropDatabase(defaultDbName);
			createDatabase(defaultDbName);

			m_db = Connection().loadDatabase(const_cast<ConnectionConfiguration&>(defaultConfiguration));
		}

		void TearDown() override
		{
			m_db.release();
			dropDatabase(defaultDbName);
		}

		std::unique_ptr<IDatabase> m_db;
	};

	TEST_F(DbForeignKeysTest, testSQLOperationsDeleteForeignKeyDeleteNoAction)
	{
		IDatabase& db = *(m_db.get());
		createPairOfTables(db, getPrefixedElement(TABLE_1, SCHEMA_PREFIX), 100, getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX), 50, "NO ACTION", "NO ACTION");

		ASSERT_THROW(m_db->executeOperation("DELETE FROM " + getPrefixedElement(TABLE_1, SCHEMA_PREFIX) + " WHERE id = 1"), std::exception);

		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM " + getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX) + " WHERE field_int_ref_t1 = 1");
		ASSERT_NE(recordSet->getRecordsCount(), 0);
	}

	TEST_F(DbForeignKeysTest, testSQLOperationsDeleteForeignKeyDeleteCascade)
	{
		IDatabase& db = *(m_db.get());
		createPairOfTables(db, getPrefixedElement(TABLE_1, SCHEMA_PREFIX), 100, getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX), 50,  "CASCADE", "NO ACTION");

		m_db->executeOperation("DELETE FROM " + getPrefixedElement(TABLE_1, SCHEMA_PREFIX) + " WHERE id = 1");

		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM " + getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX) + " WHERE field_int_ref_t1 = 1");
		ASSERT_EQ(recordSet->getRecordsCount(), 0);
	}

	TEST_F(DbForeignKeysTest, testSQLOperationsDeleteForeignKeyDeleteRestrict)
	{
		IDatabase& db = *(m_db.get());
		createPairOfTables(db, getPrefixedElement(TABLE_1, SCHEMA_PREFIX), 100, getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX), 50, "RESTRICT", "NO ACTION");

		ASSERT_THROW(m_db->executeOperation("DELETE FROM " + getPrefixedElement(TABLE_1, SCHEMA_PREFIX) + " WHERE id = 1"), std::exception);

		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM " + getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX) + " WHERE field_int_ref_t1 = 1");
		ASSERT_NE(recordSet->getRecordsCount(), 0);

	}

	TEST_F(DbForeignKeysTest, testSQLOperationsDeleteForeignKeyDeleteSetNull)
	{
		IDatabase& db = *(m_db.get());
		createPairOfTables(db, getPrefixedElement(TABLE_1, SCHEMA_PREFIX), 100, getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX), 50, "SET NULL", "NO ACTION");

		m_db->executeOperation("DELETE FROM " + getPrefixedElement(TABLE_1, SCHEMA_PREFIX) + " WHERE id = 1");

		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM " + getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX) + " WHERE field_int_ref_t1 IS NULL");
		ASSERT_NE(recordSet->getRecordsCount(), 0);
	}

	TEST_F(DbForeignKeysTest, testSQLOperationsUpdateForeignKeyUpdateNoAction)
	{
		IDatabase& db = *(m_db.get());
		createPairOfTables(db, getPrefixedElement(TABLE_1, SCHEMA_PREFIX), 100, getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX), 50, "NO ACTION", "NO ACTION");

		ASSERT_THROW(m_db->executeOperation("UPDATE " + getPrefixedElement(TABLE_1, SCHEMA_PREFIX) + " SET id = 1800 WHERE id = 1"), std::exception);

		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM " + getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX) + " WHERE field_int_ref_t1 = 1");
		ASSERT_NE(recordSet->getRecordsCount(), 0);
	}

	TEST_F(DbForeignKeysTest, testSQLOperationsUpdateForeignKeyUpdateCascade)
	{
		IDatabase& db = *(m_db.get());
		createPairOfTables(db, getPrefixedElement(TABLE_1, SCHEMA_PREFIX), 100, getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX), 50, "NO ACTION", "CASCADE");

		m_db->executeOperation("UPDATE " + getPrefixedElement(TABLE_1, SCHEMA_PREFIX) + " SET id = 1800 WHERE id = 1");

		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM "+ getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX) +" WHERE field_int_ref_t1 = 1800");
		ASSERT_NE(recordSet->getRecordsCount(), 0);

		recordSet = m_db->executeQuery("SELECT * FROM " + getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX) + " WHERE FIELD_INT_REF_T1 = 1");
		ASSERT_EQ(recordSet->getRecordsCount(), 0);
	}

	TEST_F(DbForeignKeysTest, testSQLOperationsUpdateForeignKeyUpdateRestrict)
	{
		IDatabase& db = *(m_db.get());
		createPairOfTables(db, getPrefixedElement(TABLE_1, SCHEMA_PREFIX), 100, getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX), 50, "NO ACTION", "RESTRICT");

		ASSERT_THROW(m_db->executeOperation("UPDATE " + getPrefixedElement(TABLE_1, SCHEMA_PREFIX) + " SET id = 1800 WHERE id = 1"), std::exception);

		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM " + getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX) + " WHERE field_int_ref_t1 = 1");
		ASSERT_NE(recordSet->getRecordsCount(), 0);
	}

	TEST_F(DbForeignKeysTest, testSQLOperationsUpdateForeignKeyUpdateSetNull)
	{
		IDatabase& db = *(m_db.get());		
		createPairOfTables(db, getPrefixedElement(TABLE_1, SCHEMA_PREFIX), 100, getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX), 50, "NO ACTION", "SET NULL");

		m_db->executeOperation("UPDATE " + getPrefixedElement(TABLE_1, SCHEMA_PREFIX) + " SET id = 1800 WHERE id = 1");

		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM " + getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX) + " WHERE field_int_ref_t1 IS NULL");
		ASSERT_NE(recordSet->getRecordsCount(), 0);

		recordSet = m_db->executeQuery("SELECT * FROM " + getPrefixedElement(TABLE_FOREIGN_KEYS, SCHEMA_PREFIX) + " WHERE field_int_ref_t1 = 1");
		ASSERT_EQ(recordSet->getRecordsCount(), 0);
	}
}