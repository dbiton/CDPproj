#pragma once

#include <vector>
#include <time.h>
#include <string>

class Logger
{
	struct LogData {
		double time;
		uint32_t sketch_ring_num;
		uint32_t sketch_size_byte;
		uint32_t res_actual;
		uint32_t res_sketch;
		uint32_t res_ring;
	};

	std::vector<LogData> data;
public:
	Logger();

	void log(double time, uint32_t sketch_ring_num, uint32_t sketch_size_byte, uint32_t res_actual, uint32_t res_sketch, uint32_t res_ring);
	void write(std::string path);
};

