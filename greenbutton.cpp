#include <iostream>
#include <ctime>
#include <algorithm>
#include <numeric>
#include <execution>

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>

struct MeterReading {
	MeterReading(time_t start, uint32_t reading) : start(start), reading(reading) {}
	time_t start;
	uint32_t reading; // Wh
};

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
		std::cerr << "Error parsing " << filename << " " << e.what() << "\n";
		exit(-1);
	}

	return readings;
}

int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Missing filename argument.\n";
		exit(-1);
	}

	std::string filename = argv[1];
	std::vector<MeterReading> readings = parseGreenButtonXml(filename);

	// For now just print some simple statistics...
	auto average = std::reduce(std::execution::par, readings.begin(), readings.end(), 0,
				[](int acc, MeterReading b) { return acc + b.reading; }
			) / static_cast<double>(readings.size());

	auto [min, max] = std::minmax_element(std::execution::par, readings.begin(), readings.end(),
				[](const MeterReading& a, const MeterReading& b) { return a.reading < b.reading; });

	std::cout << "Collected " << readings.size() << " readings "
			<< readings.size() / 24 << " days of data average usage "
			<< average << " minmax(" << min->reading <<", " << max->reading << ")Wh\n";

	return 0;
}
