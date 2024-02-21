#pragma once

#include "DbAdapterInterface/IConnectionConfiguration.h"

namespace systelab::db::postgresql {

	class ConnectionConfiguration : public IConnectionConfiguration
	{
	public:
		ConnectionConfiguration(const std::string& user,
								const std::string& password,
								const std::string& host,
								const std::string& port = "5432",
								const std::optional<std::string>& database = std::nullopt);
		~ConnectionConfiguration() override;

		bool hasParameter(const std::string& name) const override;
		std::string getParameter(const std::string& name) const override;

	private:
		std::map<std::string, std::string> m_parameters;
	};
}