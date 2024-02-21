#pragma once

namespace systelab::db {
	class IDatabase;
}

namespace systelab::db::postgresql::unit_test {

	// DDL operations
	void createTable(IDatabase& db, const std::string& tableName, const std::string& prefix, const unsigned int numRecords);
	void createPairOfTables(IDatabase& db, const std::string& tableName_t1, const unsigned int numRecords_t1,
							const std::string& tableName_t2, const unsigned int numRecords_t2,
							const std::string& onDelete, const std::string& onUpdate);

	void dropTable(IDatabase& db, const std::string table);

	void createDatabase(const std::string& dbName);
	void dropDatabase(const std::string& dbName);
	
	// Field values
	int getFieldIntIndexValue(unsigned int id);
	int getFieldIntNoIndexValue(unsigned int id);
	std::string getFieldStringIndexValue(unsigned int id);
	std::string getFieldStringNoIndexValue(unsigned int id);
	double getFieldRealValue(unsigned int id);
	bool getFieldBooleanValue(unsigned int id);
	std::chrono::system_clock::time_point getFieldDateValue(unsigned int id);
	std::chrono::system_clock::time_point getFieldDateBaseDate();

	// Expected records
	unsigned int getNumRecordsWithFieldIntIndexZero(unsigned int tableRecords);
	unsigned int getNumRecordsWithFieldIntNoIndexZero(unsigned int tableRecords);
	unsigned int getNumRecordsWithFieldStringIndexZero(unsigned int tableRecords);
	unsigned int getNumRecordsWithFieldStringNoIndexZero(unsigned int tableRecords);
	unsigned int getNumRecordsWithFieldRealZero(unsigned int tableRecords);
	unsigned int getNumRecordsWithFieldBoolTrue(unsigned int tableRecords);
	unsigned int getNumRecordsWithFieldDateIsBaseDate(unsigned int tableRecords);

	// Misc
	std::string getPrefixedElement(const std::string& tableName, const std::string& prefix);

}