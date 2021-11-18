#include "PgSQLDatabase.h"

#include <iostream>
#include "fmt/chrono.h"

// TODO: Account for existing data/only insert new data.  May need to parse/store the start/end from XML
bool PgSQLDatabase::insertReading(const MeterReading& r) {
	if (!m_conn.is_open())
		return false;

	pqxx::work txn {m_conn};
	try {
		txn.exec_prepared("add_reading", fmt::format("{:%FT%TZ}", fmt::localtime(r.start)), r.reading);
		txn.commit();
	}
	catch(const std::exception& e) {
		std::cerr << fmt::format("Error insterting meter reading into pgsql db: {}\n", e.what());
		txn.abort();
		return false;
	}
	return true;
}

bool PgSQLDatabase::insertBulk(MeterReadingIterator begin, MeterReadingIterator end) {
	if (!m_conn.is_open())
		return false;

	pqxx::work txn {m_conn};
	try {
		pqxx::stream_to stream {txn, "meter_readings", std::vector<std::string> {"reading_start", "usage"}};
		int i {0};
		for (auto reading = begin; reading != end; reading++) {
			stream << std::make_tuple(fmt::format("{:%FT%TZ}", fmt::localtime(reading->start)), reading->reading);
			i++;
		}
		std::cerr << fmt::format("Wrote {} rows via stream_to.\n", i);
		stream.complete();
		txn.commit();
	}
	catch(const std::exception& e) {
		std::cerr << fmt::format("Exception inserting via stream: {}\n", e.what());
		return false;
	}
	return true;
}

std::time_t PgSQLDatabase::getLastReading() {
	if (!m_conn.is_open())
		return false;

	pqxx::work txn {m_conn};
	try {
		auto res = txn.exec("");
	}
	catch(const std::exception& e) {
		std::cerr << fmt::format("Exception checking for last reading in pgsql: {}\n", e.what());
		return false;
	}
	return false; // TODO Implement me!
}
