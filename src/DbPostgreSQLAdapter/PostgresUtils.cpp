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

	bool isBooleanTrue(const std::string& postgresBoolean)
	{
		std::string lowerCaseValue = postgresBoolean; 
		std::transform(lowerCaseValue.cbegin(), lowerCaseValue.cend(), lowerCaseValue.begin(), ::tolower);

		return (lowerCaseValue == "t"
			    || lowerCaseValue == "true"
			    || lowerCaseValue == "y"
				|| lowerCaseValue == "yes"
				|| lowerCaseValue == "on"
				|| lowerCaseValue == "1"
				);
	}

	void throwPostgressException(const PGresult* statementResult, const std::source_location& srcLocation)
	{
		const std::string errorMessage = PQresultErrorMessage(statementResult);
		std::ostringstream exceptionStrem;
		exceptionStrem << "# ERR: SQLException in " << srcLocation.file_name()
			<< "(" << srcLocation.function_name() << ") on line " << srcLocation.line() << std::endl
			<< "# ERR: " << errorMessage << std::endl;
		throw std::runtime_error(exceptionStrem.str()); // Or specific exception if available
	}

}