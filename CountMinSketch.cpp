#include "CountMinSketch.h"
#include <string>
#include <cmath>

CountMinSketch::CountMinSketch(float err_prob, float err_amount) : 
	num_events(0)
{
	float EULAR_NUM = 2.71828;
	int width = std::ceil(EULAR_NUM / err_amount);
	int depth = std::ceil(log(1 / (1-err_prob)));
	sketch.create(width);
}

CountMinSketch::~CountMinSketch() {
	sketch.close();
}

void CountMinSketch::add(int e) {
	num_events++;
	std::string e_str = std::to_string(e);
	const char* e_cstr = e_str.c_str();
	size_t e_len = e_str.length();
	sketch.add(e_cstr, e_len, e);
}

float CountMinSketch::query(int e) {
	std::string e_str = std::to_string(e);
	const char* e_cstr = e_str.c_str();
	size_t e_len = e_str.length();
	return sketch.get(e_cstr, e_len);
}

unsigned CountMinSketch::numEvents() {
	return num_events;
}

std::vector<int> CountMinSketch::collectAll() {
	return std::vector<int>();
}

std::vector<int> CountMinSketch::collectHeavyHitters() {
	return std::vector<int>();
}