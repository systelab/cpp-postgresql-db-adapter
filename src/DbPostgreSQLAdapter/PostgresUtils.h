#pragma once

typedef struct pg_result PGresult;

namespace systelab::db::postgresql::utils {

	typedef std::unique_ptr<PGresult, void(*)(PGresult*)> PGResultRAII;

	PGResultRAII createRAIIPGresult(PGresult* result);

	std::chrono::system_clock::time_point stringISOToDateTime(const std::string& dateTime);
	std::string dateTimeToISOString(const std::chrono::system_clock::time_point& dateTime);
	bool isDateTimeNull(const std::chrono::system_clock::time_point& dateTime);

	bool isBooleanTrue(const std::string& postgresBoolean);
}