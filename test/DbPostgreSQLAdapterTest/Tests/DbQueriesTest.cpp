#include "stdafx.h"

#include "Connection.h"
#include "ConnectionConfiguration.h"
#include "DbAdapterInterface/IDatabase.h"
#include "DbAdapterInterface/IRecordSet.h"
#include "DbAdapterInterface/ITable.h"
#include "DbAdapterInterface/ITableRecordSet.h"
#include "DbAdapterInterface/ITransaction.h"
#include "Helpers/Helpers.h"
#include "Helpers/DefaultConnectionConfiguration.h"

namespace {
	static const std::string SCHEMA_PREFIX = "public";
	static const std::string EVALUATION_TABLE_NAME = "TESTS";

	std::string dateTimeToISOString(const std::chrono::system_clock::time_point& dateTime)
	{
		if (dateTime != std::chrono::system_clock::time_point{})
		{
			return std::format("{:%F %T%z}", dateTime);
		}

		return "";
	}
}

using namespace testing;
using namespace std::chrono_literals;
namespace systelab::db::postgresql::unit_test {

	class DbQueriesTest: public Test
	{
	public:
		void SetUp()
		{
			dropDatabase(defaultDbName);
			createDatabase(defaultDbName);

			m_db = Connection().loadDatabase(const_cast<ConnectionConfiguration&>(defaultConfiguration));
			createTable();
			startTimeTracking();
		}

		void TearDown()
		{
			endTimeTracking();
		}

	protected:
		std::unique_ptr<IDatabase> m_db;
		std::chrono::system_clock::time_point m_startTime;

		void startTimeTracking()
		{
			m_startTime = std::chrono::system_clock::now();
		}

		void endTimeTracking()
		{
			std::chrono::system_clock::time_point endTime = std::chrono::system_clock::now();
			std::cout << "\nExecution time: " << std::chrono::duration_cast<std::chrono::milliseconds>(endTime - m_startTime) << " ms.\n";
		}

		void createTable()
		{
			m_db->executeOperation("CREATE TABLE public.\"TESTS\" (ID INT PRIMARY KEY NOT NULL, FIELD_INT_INDEX INT, FIELD_INT_NO_INDEX INT, FIELD_STR_INDEX VARCHAR(255), FIELD_STR_NO_INDEX VARCHAR(255), FIELD_DATE TIMESTAMP WITH TIME ZONE DEFAULT NULL)");
			m_db->executeOperation("CREATE INDEX INT_INDEX ON public.\"TESTS\" (FIELD_INT_INDEX)");
			m_db->executeOperation("CREATE INDEX STR_INDEX ON public.\"TESTS\" (FIELD_STR_INDEX)");

			std::unique_ptr<ITransaction> transaction = m_db->startTransaction();

			std::chrono::system_clock::time_point today(std::chrono::sys_days{ 1d / 1 / 2015 });
			for (unsigned int i = 0; i < 10000; i++)
			{
				std::ostringstream oss;
				const auto currentDate = today + std::chrono::hours(7 * i);
				std::string strDate = utils::dateTimeToISOString(currentDate);
				oss << "INSERT INTO public.\"TESTS\" (ID, FIELD_INT_INDEX, FIELD_INT_NO_INDEX, FIELD_STR_INDEX, FIELD_STR_NO_INDEX, FIELD_DATE) VALUES (" 
					<< i 
					<< ", " << i%7 
					<< ", " << i%10 
					<< ", \'STR" << i%9 << "\'"
					<< ", \'STR" << i%12 << "\'"
					<< ", '" << strDate << "'"
					<< ") RETURNING ID";
				m_db->executeOperation(oss.str());
			}

			transaction->commit();
			transaction.reset();
		}

		unsigned int iterateTableRecordset(ITableRecordSet& recordSet)
		{
			unsigned int nRecords = 0;
			while (recordSet.isCurrentRecordValid())
			{
				const ITableRecord& record = recordSet.getCurrentRecord();
				++nRecords;
				recordSet.nextRecord();
			}

			return nRecords;
		}
		unsigned int iterateRecordset(IRecordSet& recordSet)
		{
			unsigned int nRecords = 0;
			while ( recordSet.isCurrentRecordValid() )
			{
				const IRecord& record = recordSet.getCurrentRecord();
				++nRecords;
				recordSet.nextRecord();
			}

			return nRecords;
		}

		std::unique_ptr<IDatabase> m_db;
		std::chrono::system_clock::time_point m_startTime;
	};

	TEST_F(DbQueriesTest, testQueryAll)
	{
		ITable& table = m_db->getTable(getPrefixedElement(EVALUATION_TABLE_NAME, SCHEMA_PREFIX));
		std::unique_ptr<ITableRecordSet> recordSet = table.getAllRecords();
		ASSERT_EQ(10000, recordSet->getRecordsCount());
		iterateTableRecordset(*recordSet);
	}

	TEST_F(DbQueriesTest, testQueryRowPrimaryKey)
	{
		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM public.\"TESTS\" WHERE id = 50");
		ASSERT_EQ(1, recordSet->getRecordsCount());
		iterateRecordset(*recordSet);
	}

	TEST_F(DbQueriesTest, testQueryRowsNonIndexedIntField)
	{
		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM public.\"TESTS\" WHERE FIELD_INT_NO_INDEX = 5");
		ASSERT_EQ(1000, recordSet->getRecordsCount());
		iterateRecordset(*recordSet);
	}

	TEST_F(DbQueriesTest, testQueryRowsIndexedIntField)
	{
		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM public.\"TESTS\" WHERE FIELD_INT_INDEX = 5");
		ASSERT_EQ(1428, recordSet->getRecordsCount());
		iterateRecordset(*recordSet);
	}

	TEST_F(DbQueriesTest, testQueryRowsNonIndexedStringField)
	{
		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM public.\"TESTS\" WHERE FIELD_STR_NO_INDEX = 'STR4'");
		ASSERT_EQ(833, recordSet->getRecordsCount());
		iterateRecordset(*recordSet);
	}

	TEST_F(DbQueriesTest, testQueryRowsIndexedStringField)
	{
		std::unique_ptr<IRecordSet> recordSet = m_db->executeQuery("SELECT * FROM public.\"TESTS\" WHERE FIELD_STR_INDEX = 'STR6'");
		ASSERT_EQ(1111, recordSet->getRecordsCount());
		iterateRecordset(*recordSet);
	}

	TEST_F(DbQueriesTest, testInsertSingleRow)
	{
		m_db->executeOperation("INSERT INTO public.\"TESTS\" (ID, FIELD_INT_INDEX, FIELD_INT_NO_INDEX, FIELD_STR_INDEX, FIELD_STR_NO_INDEX) VALUES (10001, 5, 5, 'STR5', 'STR5') RETURNING ID");
	}

	TEST_F(DbQueriesTest, testInsert1000RowsNoTransaction)
	{
		std::chrono::system_clock::time_point today(std::chrono::sys_days{ 1d / 1 / 2018 });
		for (unsigned int i = 10001; i < 11000; i++)
		{
			std::ostringstream oss;
			const auto currentDate = today + std::chrono::hours(7 * i);
			std::string strDate = utils::dateTimeToISOString(currentDate);
			oss << "INSERT INTO public.\"TESTS\" (ID, FIELD_INT_INDEX, FIELD_INT_NO_INDEX, FIELD_STR_INDEX, FIELD_STR_NO_INDEX) VALUES (" 
				<< i 
				<< ", " << i%7 
				<< ", " << i%10 
				<< ", 'STR" << i%9 << "'"
				<< ", 'STR" << i%12 << "'"
				<< ") RETURNING ID";
			m_db->executeOperation(oss.str());
		}
	}

	TEST_F(DbQueriesTest, testInsert1000RowsTransaction)
	{
		std::unique_ptr<ITransaction> transaction = m_db->startTransaction();
		std::chrono::system_clock::time_point today(std::chrono::sys_days{ 1d / 1 / 2018 });
		for (unsigned int i = 10001; i < 11000; i++)
		{
			std::ostringstream oss;
			const auto currentDate = today + std::chrono::hours(7 * i);
			std::string strDate = utils::dateTimeToISOString(currentDate);
			oss << "INSERT INTO public.\"TESTS\" (ID, FIELD_INT_INDEX, FIELD_INT_NO_INDEX, FIELD_STR_INDEX, FIELD_STR_NO_INDEX) VALUES (" 
				<< i 
				<< ", " << i%7 
				<< ", " << i%10 
				<< ", 'STR" << i%9 << "'"
				<< ", 'STR" << i%12 << "'"
				<< ") RETURNING ID";
			m_db->executeOperation(oss.str());
		}

		transaction->commit();
		transaction.reset();
	}

	TEST_F(DbQueriesTest, testUpdateIntFieldNotIndexed)
	{
		m_db->executeOperation("UPDATE public.\"TESTS\" SET FIELD_INT_NO_INDEX = 100 WHERE ID = 5");
	}

	TEST_F(DbQueriesTest, testUpdateIntFieldIndexed)
	{
		m_db->executeOperation("UPDATE public.\"TESTS\" SET FIELD_INT_INDEX = 100 WHERE ID = 5");
	}

	TEST_F(DbQueriesTest, testUpdateStringFieldNotIndexed1Row)
	{
		m_db->executeOperation("UPDATE public.\"TESTS\" SET FIELD_STR_NO_INDEX = 'STR100' WHERE ID = 5");
	}

	TEST_F(DbQueriesTest, testUpdateStringFieldIndexed1Row)
	{
		m_db->executeOperation("UPDATE public.\"TESTS\" SET FIELD_STR_INDEX = 'STR100' WHERE ID = 5");
	}

	TEST_F(DbQueriesTest, testUpdateStringFieldNotIndexed1000Row)
	{
		m_db->executeOperation("UPDATE public.\"TESTS\" SET FIELD_STR_NO_INDEX = 'STR100'");
	}

	TEST_F(DbQueriesTest, testUpdateStringFieldIndexed1000Row)
	{
		m_db->executeOperation("UPDATE public.\"TESTS\" SET FIELD_STR_INDEX = 'STR100'");
	}

	TEST_F(DbQueriesTest, testUpdateStringFieldNotIndexed1000RowDifferentValues)
	{
		for (unsigned int i = 0; i < 1000; i++)
		{
			std::ostringstream oss;
			oss << "UPDATE public.\"TESTS\" SET FIELD_STR_NO_INDEX = "
				<< "'STR" << (i + 1) << "'"
				<< " WHERE ID = " << i;
			m_db->executeOperation(oss.str());
		}
	}

	TEST_F(DbQueriesTest, testUpdateStringFieldIndexed1000RowDifferentValues)
	{
		std::unique_ptr<ITransaction> transaction = m_db->startTransaction();

		for (unsigned int i = 0; i < 1000; i++)
		{
			std::ostringstream oss;
			oss << "UPDATE public.\"TESTS\" SET FIELD_STR_INDEX = "
				<< "'STR" << (i + 1) << "'"
				<< " WHERE ID = " << i;
			m_db->executeOperation(oss.str());
		}

		transaction->commit();
		transaction.reset();
	}

	TEST_F(DbQueriesTest, testDeleteRecord)
	{
		m_db->executeOperation("DELETE FROM public.\"TESTS\" WHERE ID = 5");
	}

	TEST_F(DbQueriesTest, testDelete1000RecordsNoTransaction)
	{
		for (unsigned int i = 0; i < 1000; i++)
		{
			std::ostringstream oss;
			oss << "DELETE FROM public.\"TESTS\" TESTS WHERE ID = "
				<< i;
			m_db->executeOperation(oss.str());
		}
	}

	TEST_F(DbQueriesTest, testDelete1000RecordsTransaction)
	{
		std::unique_ptr<ITransaction> transaction = m_db->startTransaction();

		for (unsigned int i = 0; i < 1000; i++)
		{
			std::ostringstream oss;
			oss << "DELETE FROM public.\"TESTS\" WHERE ID = "
				<< i;
			m_db->executeOperation(oss.str());
		}

		transaction->commit();
		transaction.reset();
	}

}

