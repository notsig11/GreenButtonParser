#include <iostream>
#include <ctime>
#include <algorithm>
#include <numeric>
#include <execution>

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>

#include "fmt/core.h"

#include "greenbutton.h"
#include "PgSQLDatabase.h"

std::vector<MeterReading> parseGreenButtonXml(std::string filename) {
	std::vector<MeterReading> readings;

	try {
		rapidxml::file<> infile(filename.c_str());

		rapidxml::xml_document<> meter_reads;
		meter_reads.parse<0>(infile.data());
		rapidxml::xml_node<>* root = meter_reads.first_node();
		rapidxml::xml_node<>* reading = root->last_node("entry");
		rapidxml::xml_node<>* content = reading->first_node("content");

		for (auto intervalBlock = content->first_node("IntervalBlock"); intervalBlock;
					intervalBlock = intervalBlock->next_sibling()) {
			// TODO: Figure out what to do with interval level duration/start time.  Does it make sense?
//			auto interval = intervalBlock->first_node("interval");
//			std::cout << "Interval Block duration: " << interval->first_node("duration")->value()
//										<< " Interval start time: " << interval->first_node("start")->value() << "\n";

			for (auto intervalReading = intervalBlock->first_node("IntervalReading");
						intervalReading; intervalReading = intervalReading->next_sibling()) {
				auto startTime = intervalReading->first_node("timePeriod")->first_node("start");
				auto reading = intervalReading->first_node("value");

				readings.emplace_back(std::stol(startTime->value()), std::stoi(reading->value()));
			}
		}
	}
	catch (rapidxml::parse_error &e) {
		std::cerr << fmt::format("Error parsing {}: {}\n", filename, e.what());
		exit(-1);
	}

	return readings;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Missing filename argument.\n";
		exit(-1);
	}

	PgSQLDatabase db("host=localhost port=5433 dbname=energy_usage user=greenbutton password=greenbutton");
	std::string filename = argv[1];
	std::vector<MeterReading> readings = parseGreenButtonXml(filename);

	// For now just print some simple statistics... (parallel processed for fun!)
	auto average = std::reduce(std::execution::par, readings.begin(), readings.end(), 0,
				[](int acc, MeterReading b) { return acc + b.reading; }
			) / static_cast<double>(readings.size());

	auto [min, max] = std::minmax_element(std::execution::par, readings.begin(), readings.end(),
				[](const MeterReading& a, const MeterReading& b) { return a.reading < b.reading; });

	db.insertBulk(readings.begin(), readings.end());

	std::cout << fmt::format("Stored {} readings ({} days of data).  Average usage {:.3f}Wh, max {}Wh\n",
			readings.size(), readings.size() / 24, average, max->reading);

	return 0;
}
