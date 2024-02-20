#include "stdafx.h"

#include "Connection.h"
#include "ConnectionConfiguration.h"
#include "DbAdapterInterface/IDatabase.h"
#include "Helpers/Helpers.h"
#include "Helpers/DefaultConnectionConfiguration.h"


using namespace testing;
namespace systelab::db::postgresql::unit_test {

	class DbConnectionTest : public Test
	{
		void SetUp() override
		{
			dropDatabase(defaultDbName);
			createDatabase(defaultDbName);
		}
	};

	TEST_F(DbConnectionTest, testLoadDatabaseForHappyPathReturnsDatabaseObject)
	{
		Connection connection;
		auto database = Connection().loadDatabase(const_cast<ConnectionConfiguration&>(defaultConfiguration));

		ASSERT_THAT(database, NotNull());
	}

	TEST_F(DbConnectionTest, testLoadDatabaseWithWrongCredentialsThrowsPostgreSQLException)
	{
		const std::string dbName = "postgres";
		const std::string dbHost = "localhost";
		const std::string dbUser = "neo";
		const std::string dbPassword = "letmein";
		const std::string dbPort = "5432";

		postgresql::ConnectionConfiguration configuration(dbUser, dbPassword, dbHost, dbPort, dbName);
		ASSERT_THROW(Connection().loadDatabase(configuration), postgresql::Connection::PostgreSQLException);
	}
}
