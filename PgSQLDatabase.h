#pragma once

#include <ctime>
#include <pqxx/pqxx>

#include "greenbutton.h"


class PgSQLDatabase {
using MeterReadingIterator = std::vector<MeterReading>::const_iterator;

public:
  PgSQLDatabase(const std::string& connectionString) : m_conn(connectionString) {
		m_conn.prepare("add_reading", "INSERT INTO meter_readings (reading_start, usage) VALUES($1, $2);");
	}

	bool insertReading(const MeterReading& r);
	bool insertBulk(MeterReadingIterator begin, MeterReadingIterator end);

	std::pair<std::time_t, std::time_t> getStoredRange();

private:
	pqxx::connection m_conn;
};
