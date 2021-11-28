#include "PgSQLDatabase.h"

#include <iostream>
#include "fmt/chrono.h"

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

	// Get date range we already have stored
	auto [min, max] = getStoredRange();

	pqxx::work txn {m_conn};
	try {
		pqxx::stream_to stream {txn, "meter_readings", std::vector<std::string> {"reading_start", "usage"}};
		int i {0};
		for (auto reading = begin; reading != end; reading++) {
			if (reading->start > max || reading->start < min) {
				stream << std::make_tuple(fmt::format("{:%FT%T%z}", fmt::localtime(reading->start)), reading->reading);
				i++;
			}
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

std::pair<std::time_t, std::time_t> PgSQLDatabase::getStoredRange() {
	if (!m_conn.is_open())
		return std::make_pair(0, 0);

	pqxx::work txn {m_conn};
	try {
		// DO NOT use timezone here!  Postgres gives us the epoch at the stored TZ which is what the XML gives us.
		auto res = txn.exec("SELECT EXTRACT(EPOCH FROM MIN(reading_start)) AS start, \
																EXTRACT(EPOCH FROM MAX(reading_start)) AS end \
																FROM meter_readings;");
		return std::make_pair(res[0]["start"].as<uint64_t>(), res[0]["end"].as<uint64_t>());
	}
	catch(const std::exception& e) {
		std::cerr << fmt::format("Exception checking for last reading in pgsql: {}\n", e.what());
		return std::make_pair(0, 0);
	}
}
