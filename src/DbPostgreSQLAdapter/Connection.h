#pragma once

#include "DbAdapterInterface/IConnection.h"

namespace systelab {
	namespace db {
		class IDatabase;
	}
}

namespace systelab { namespace db { namespace postgresql {

	class Connection : public IConnection
	{
	public:
		Connection();
		~Connection() override = default;

		std::unique_ptr<IDatabase> loadDatabase(IConnectionConfiguration&) override;

	public:
		struct PostgreSQLException : public Exception
		{
			PostgreSQLException(const std::string& message, const std::string& extendedMessage)
				: Exception(message + ": " + extendedMessage)
			{}
		};
	};
}}}