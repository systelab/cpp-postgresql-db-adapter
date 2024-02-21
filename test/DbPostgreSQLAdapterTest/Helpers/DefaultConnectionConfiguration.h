#pragma once
#include "ConnectionConfiguration.h"

namespace systelab::db::postgresql::unit_test {
	static const std::string defaultDbName = "dummyDB";
	static const std::string defaultDbHost = "localhost";
	static const std::string defaultDbUser = "testUser";
	static const std::string defaultDbPassword = "testPassword";
	static const std::string defaultDbPort = "5432";

	static const ConnectionConfiguration defaultConfiguration(defaultDbUser, defaultDbPassword, defaultDbHost, defaultDbPort, defaultDbName);
}
