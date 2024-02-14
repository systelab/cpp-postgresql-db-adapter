#include "stdafx.h"
#include "Transaction.h"

#include "DbAdapterInterface/IDatabase.h"

namespace systelab::db::postgresql {

	Transaction::Transaction(IDatabase& database)
		: m_database(database)
	{
		std::string operation = "BEGIN TRANSACTION";
		m_database.executeOperation(operation);
	}

	void Transaction::commit()
	{
		std::string operation = "END";
		m_database.executeOperation(operation);
	}

	void Transaction::rollback()
	{
		std::string operation = "ROLLBACK";
		m_database.executeOperation(operation);
	}
}