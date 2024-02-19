#include "stdafx.h"
#include "PostgresUtils.h"

namespace systelab::db::postgresql::utils {
	PGResultRAII createRAIIPGresult(PGresult* result)
	{
		return std::unique_ptr<PGresult, void(*)(PGresult*)>(result, PQclear);
	}

	std::chrono::system_clock::time_point stringISOToDateTime(const std::string& dateTime)
	{
		std::chrono::system_clock::time_point timePointDateTime;
		std::istringstream{ dateTime } >> std::chrono::parse("%F %T%z", timePointDateTime);
		return timePointDateTime;
	}

	std::string dateTimeToISOString(const std::chrono::system_clock::time_point& dateTime)
	{
		if (dateTime != std::chrono::system_clock::time_point{})
		{
			return std::format("{:%F %T%z}", dateTime);
		}

		return "";
	}

	bool isDateTimeNull(const std::chrono::system_clock::time_point& dateTime)
	{
		return dateTime == std::chrono::system_clock::time_point();
	}

}