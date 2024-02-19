#include "stdafx.h"

#include "Database.h"
#include "DbAdapterInterface/ITable.h"
#include "PostgresUtils.h"
#include "RecordSet.h"
#include "Table.h"
#include "TableRecordSet.h"
#include "Transaction.h"

namespace systelab::db::postgresql {

	Database::Database(PGconn* database)
		: m_database(database)
		, m_lastOperationRowsAffected(0)
		, m_lastInsertedRowId(0)
	{
	}

	Database::~Database()
	{
		PQfinish(m_database);
	}

	ITable& Database::getTable(const std::string& tableName)
	{
		const auto itr = m_tables.find(tableName);
		if (itr == m_tables.cend())
		{
			m_tables.insert(std::make_pair(tableName, new Table(*this, tableName)));
		}

		const auto& table = m_tables.at(tableName);
		return *(table.get());
	}

	std::unique_ptr<IRecordSet> Database::executeQuery(const std::string& query)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		const auto statementResult = utils::createRAIIPGresult(PQexec(m_database, query.c_str()));
		if (PQresultStatus(statementResult.get()) == PGRES_TUPLES_OK)
		{
			return std::make_unique<RecordSet>(statementResult.get());
		}
		else
		{
			const std::string errorMessage = PQresultErrorMessage(statementResult.get());
			
			std::ostringstream exceptionStrem;
			exceptionStrem << "# ERR: SQLException in " << __FILE__;
			exceptionStrem << "(Database::executeQuery) on line " << __LINE__ << '\n';
			exceptionStrem << "# ERR: " << errorMessage << std::endl;

			throw std::runtime_error(exceptionStrem.str());
		}
	}

	std::unique_ptr<ITableRecordSet> Database::executeTableQuery(const std::string& query, ITable& table)
	{	
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		const auto statementResult = utils::createRAIIPGresult(PQexec(m_database, query.c_str()));
		if (PQresultStatus(statementResult.get()) == PGRES_TUPLES_OK)
		{
			return std::make_unique<TableRecordSet>(table, statementResult.get());
		}
		else
		{
			const std::string errorMessage = PQresultErrorMessage(statementResult.get());

			std::ostringstream exceptionStrem;
			exceptionStrem << "# ERR: SQLException in " << __FILE__;
			exceptionStrem << "(Database::executeOperation) on line " << __LINE__ << '\n';
			exceptionStrem << "# ERR: " << errorMessage << std::endl;

			throw std::runtime_error(exceptionStrem.str());
		}

	}

	void Database::executeOperation(const std::string& operation)
	{
		std::lock_guard<std::recursive_mutex> lock(m_mutex);
		const auto statementResult = utils::createRAIIPGresult(PQexec(m_database, operation.c_str()));
		const auto result = PQresultStatus(statementResult.get());
		if (result == PGRES_TUPLES_OK)
		{
			// Retrieve last inserted id. Requires that the query returns the id
			const unsigned int rowsCount = static_cast<unsigned int>(PQntuples(statementResult.get()));
			const std::string rowId = PQgetvalue(statementResult.get(), 0, rowsCount-1);
			m_lastInsertedRowId = std::stoi(rowId);
		}
		else if (result != PGRES_COMMAND_OK)
		{
			const std::string errorMessage = PQresultErrorMessage(statementResult.get());

			std::ostringstream exceptionStrem;
			exceptionStrem << "# ERR: SQLException in " << __FILE__;
			exceptionStrem << "(Database::executeOperation) on line " << __LINE__ << '\n';
			exceptionStrem << "# ERR: " << errorMessage << std::endl;

			throw std::runtime_error(exceptionStrem.str());
		}

		m_lastOperationRowsAffected = std::atoi(PQcmdTuples(statementResult.get()));
	}

	void Database::executeMultipleStatements(const std::string& statements)
	{
		throw std::runtime_error("Not implemented yet");
	}

	RowsAffected Database::getRowsAffectedByLastChangeOperation() const
	{
		return m_lastOperationRowsAffected;
	}

	RowId Database::getLastInsertedRowId() const
	{
		return m_lastInsertedRowId;
	}

	std::unique_ptr<ITransaction> Database::startTransaction()
	{
		return std::make_unique<Transaction>(*this);
	}
}