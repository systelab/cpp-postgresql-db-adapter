#include "stdafx.h"
#include "ConnectionConfiguration.h"

namespace systelab::db:: postgresql {

	ConnectionConfiguration::ConnectionConfiguration(const std::string& user,
													 const std::string& password,
													 const std::string& host,
													 const std::string& port,
													 const std::optional<std::string>& database)
		: m_parameters({{ "host", host }, { "user", user }, { "password", password }, { "port", port }})
	{
		if(database.has_value())
		{
			m_parameters.insert({ "database", database.value() });
		}
	}

	ConnectionConfiguration::~ConnectionConfiguration() = default;

	bool ConnectionConfiguration::hasParameter(const std::string& parameterName) const
	{
		return m_parameters.contains(parameterName);
	}

	std::string ConnectionConfiguration::getParameter(const std::string& parameterName) const
	{
		if (hasParameter(parameterName))
		{
			return m_parameters.at(parameterName);
		}
		
		return "";	
	}
}

