#include "stdafx.h"
#include "Connection.h"

#include "Database.h"

#include "libpq-fe.h"

namespace {
	std::unique_ptr<PGresult, void(*)(PGresult*)> makePGResult(PGresult* result)
	{
		return std::unique_ptr<PGresult, void(*)(PGresult*)>(result, PQclear);
	}
}

namespace systelab { namespace db { namespace postgresql {

	Connection::Connection()
	{}

	std::unique_ptr<IDatabase> Connection::loadDatabase(IConnectionConfiguration& configuration)
	{
		PGconn* connection;
		const std::string host = configuration.getParameter("host");
		const std::string port = configuration.getParameter("port");
		const std::string user = configuration.getParameter("user");
		const std::string password = configuration.getParameter("password");
		boost::optional<std::string> dbName;
		if (configuration.hasParameter("database"))
		{
			dbName = configuration.getParameter("database");
		}

		connection = PQsetdbLogin(host.data(), port.data(), nullptr, nullptr, (dbName ? dbName->data() : nullptr), user.data(), password.data());
		if (PQstatus(connection) != CONNECTION_OK)
		{
			const std::string extendedMessage = PQerrorMessage(connection);
			throw PostgreSQLException("Unable to connect to database '" + (dbName ? *dbName : "") + "'", extendedMessage);
		}

		/* Set always-secure search path, so malicious users can't take control. */
		auto result = makePGResult(PQexec(connection, "SELECT pg_catalog.set_config('search_path', '', false)"));
		if (PQresultStatus(result.get()) != PGRES_TUPLES_OK)
		{
			const std::string extendedMessage = PQerrorMessage(connection);
			throw PostgreSQLException("Unable to connect to database '" + (dbName ? *dbName : "") + "'", extendedMessage);
		}
	
		return std::make_unique<Database>(connection);
	}
}}}

