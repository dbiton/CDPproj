#include "Logger.h"
#include <iostream>
#include <fstream>


Logger::Logger()
{
}

void Logger::log(double time, uint32_t sketch_ring_num, uint32_t sketch_size_byte, uint32_t res_actual, uint32_t res_sketch, uint32_t res_ring)
{
	data.push_back({ time, sketch_ring_num, sketch_size_byte, res_actual, res_sketch, res_ring });
}

void Logger::write(std::string path)
{
	std::ofstream file;
	file.open(path);
	file << "T,RING_SIZE,RING_BYTES,RES_ACT,RES_SKETCH,RES_RING" << std::endl;
	for (auto& d : data) {
		file << d.time <<"," << d.sketch_ring_num << "," << d.sketch_size_byte <<"," << d.res_actual << "," << d.res_sketch <<","<<d.res_ring<< std::endl;
	}
	file.close();
}
