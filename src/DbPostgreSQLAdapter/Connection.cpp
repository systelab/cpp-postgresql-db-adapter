#include "stdafx.h"
#include "Connection.h"

#include "ConnectionConfiguration.h"
#include "Database.h"

#include "libpq-fe.h"
#include "PostgresUtils.h"

namespace systelab::db::postgresql {

	std::unique_ptr<IDatabase> Connection::loadDatabase(IConnectionConfiguration& configuration)
	{
		PGconn* connection;
		const std::string host = configuration.getParameter("host");
		const std::string port = configuration.getParameter("port");
		const std::string user = configuration.getParameter("user");
		const std::string password = configuration.getParameter("password");
		std::optional<std::string> dbName;
		if (configuration.hasParameter("database"))
		{
			dbName = configuration.getParameter("database");
		}

		connection = PQsetdbLogin(host.c_str(), port.c_str(), nullptr, nullptr, (dbName ? dbName->c_str() : nullptr), user.c_str(), password.c_str());
		if (PQstatus(connection) != CONNECTION_OK)
		{
			const std::string extendedMessage = PQerrorMessage(connection);
			throw PostgreSQLException("Unable to connect to database '" + (dbName ? *dbName : "") + "'", extendedMessage);
		}

		/* Set always-secure search path, so malicious users can't take control. */
		auto result = utils::createRAIIPGresult(PQexec(connection, "SELECT pg_catalog.set_config('search_path', '', false)"));
		if (PQresultStatus(result.get()) != PGRES_TUPLES_OK)
		{
			const std::string extendedMessage = PQerrorMessage(connection);
			throw PostgreSQLException("Unable to connect to database '" + (dbName ? *dbName : "") + "'", extendedMessage);
		}
	
		return std::make_unique<Database>(connection);
	}
}

