#include "stdafx.h"
#include "Connection.h"

#include "Database.h"

#include "libpq-fe.h"

namespace {
	template<typename T>
	using deleted_unique_ptr = std::unique_ptr < T, std::function<void(T*)>;
	deleted_unique_ptr<PGresult> makePGResult(const PGresult* result)
	{
		return deleted_unique_ptr<PGresult>(result, PQclear);
	}
}

namespace systelab { namespace db { namespace postgresql {

	Connection::Connection()
	{}

	std::unique_ptr<IDatabase> Connection::loadDatabase(IConnectionConfiguration& configuration)
	{
		PGconn* connection;
		const char* url = configuration.getParameter("url").data();
		const char* port = configuration.getParameter("port").data();
		const char* user = configuration.getParameter("user").data();
		const char* password = configuration.getParameter("password").data();
		const char* dbName = configuration.hasParameter("database") ? configuration.getParameter("database").data() : nullptr;

		connection = PQsetdbLogin(url, port, nullptr, nullptr, dbName , user, password);
		if (PQstatus(connection) != CONNECTION_OK)
		{
			const std::string extendedMessage = PQerrorMessage(connection);
			throw PostgreSQLException("Unable to connect to database '" + std::string(dbName) + "'", extendedMessage);
		}

		/* Set always-secure search path, so malicious users can't take control. */
		auto result = makePGResult(PQexec(connection, "SELECT pg_catalog.set_config('search_path', '', false)"));
		if (PQresultStatus(result.get()) != PGRES_TUPLES_OK)
		{
			const std::string extendedMessage = PQerrorMessage(connection);
			throw PostgreSQLException("Unable to connect to database '" + std::string(dbName) + "'", extendedMessage);
		}
	
		return std::make_unique<Database>(connection);
	}
}}}

