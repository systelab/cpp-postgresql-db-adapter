#include "stdafx.h"
#include "Transaction.h"

#include "DbAdapterInterface/IDatabase.h"

namespace systelab::db::postgresql {

	Transaction::Transaction(IDatabase& database)
		: m_database(database)
	{
		m_database.executeOperation("BEGIN TRANSACTION");
	}

	void Transaction::commit()
	{
		m_database.executeOperation("END");
	}

	void Transaction::rollback()
	{
		m_database.executeOperation("ROLLBACK");
	}
}