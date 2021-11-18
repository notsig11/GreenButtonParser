#pragma once

struct MeterReading {
	MeterReading(time_t start, uint32_t reading) : start(start), reading(reading) {}
	time_t start;
	uint32_t reading; // Wh
};

