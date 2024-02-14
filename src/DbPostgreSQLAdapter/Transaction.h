#pragma once

#include "DbAdapterInterface/ITransaction.h"

namespace systelab::db {
	class IDatabase;
}

namespace systelab::db::postgresql {

	class Transaction : public ITransaction
	{
	public:
		Transaction(IDatabase& database);
		~Transaction() override = default;

		void commit() override;
		void rollback() override;

	private:
		IDatabase& m_database;
	};
}