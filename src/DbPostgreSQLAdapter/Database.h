#pragma once

#include "DbAdapterInterface/IDatabase.h"
#include "DbAdapterInterface/ITable.h"

namespace systelab::db {
	class IRecordSet;
	class ITable;
	class ITableRecordSet;
	class ITransaction;
}

typedef struct pg_conn PGconn;

namespace systelab::db::postgresql {

	class Database : public IDatabase
	{
	public:
		Database(PGconn* database);
		~Database() override;

		ITable& getTable(const std::string& tableName) override;
		std::unique_ptr<IRecordSet> executeQuery(const std::string& query) override;
		std::unique_ptr<ITableRecordSet> executeTableQuery(const std::string& query, ITable& table);
		void executeOperation(const std::string& operation) override;
		void executeMultipleStatements(const std::string& statements) override;
		RowsAffected getRowsAffectedByLastChangeOperation() const override;
		RowId getLastInsertedRowId() const override;
		std::unique_ptr<ITransaction> startTransaction() override;

	private:
		PGconn* m_database;
		std::map<std::string, std::unique_ptr<ITable>> m_tables;
		std::recursive_mutex m_mutex;
		RowsAffected m_lastOperationRowsAffected;
		RowId m_lastInsertedRowId;
	};
}