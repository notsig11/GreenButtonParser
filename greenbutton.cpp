#include <iostream>
#include <ctime>
#include <numeric>
#include <execution>

#include <rapidxml/rapidxml.hpp>
#include <rapidxml/rapidxml_utils.hpp>

struct MeterReading {
	MeterReading(time_t p_start, uint32_t p_reading) : start(p_start), reading(p_reading) {}
	time_t start;	
	uint32_t reading; // Wh

	uint32_t operator+(const MeterReading& rhs) const { return reading + rhs.reading; }
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
			auto interval = intervalBlock->first_node("interval");
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
	catch (rapidxml::parse_error e) {
		std::cerr << "Error parsing " << filename << " " << e.what() << "\n";
		exit(-1);
	}
	
	return readings;
}

int main(int argc, char* argv[]) {
	std::string filename = argv[1];
	std::vector<MeterReading> readings = parseGreenButtonXml(filename);

/*
	for (const auto reading : readings) {
		std::cout << "time: " << std::asctime(std::localtime(&reading.start)) 
				<< reading.reading << "Wh\n";
	}
	*/
	float average;
	for (auto r : readings) { average += r.reading; }
	average /= readings.size();

	std::cout << "Collected " << readings.size() << " readings " << readings.size() / 24 << " days of data average usage " << average << "Wh\n";

	return 0;
}


