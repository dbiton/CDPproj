#include "CountMinSketch.h"
#include <string>
#include <cmath>

CountMinSketch::CountMinSketch(float _err_amount) : 
	num_events(0),
	err_amount(_err_amount)
{
	float EULAR_NUM = 2.71828;
	int width = std::ceil(EULAR_NUM / err_amount);
	sketch.create(width, UINT32_MAX);
}

CountMinSketch::~CountMinSketch() {
	sketch.close();
}

void CountMinSketch::add(int e) {
	num_events++;
	std::string e_str = std::to_string(e);
	const char* e_cstr = e_str.c_str();
	size_t e_len = e_str.length();
	sketch.inc(e_cstr, e_len);
}

float CountMinSketch::query(int e) const{
	std::string e_str = std::to_string(e);
	const char* e_cstr = e_str.c_str();
	size_t e_len = e_str.length();
	return sketch.get(e_cstr, e_len);
}

unsigned CountMinSketch::numEvents() const {
	return num_events;
}

void CountMinSketch::merge(const CountMinSketch& o)
{
	unsigned o_num_events = o.numEvents();
	unsigned tot_num_events = o_num_events + numEvents();
	num_events = tot_num_events;
	sketch.merge(o.sketch);
}

void CountMinSketch::filter(sketchFilter sf)
{
	num_events /= 2;
	sketch.filter(sf);
}

CountMinSketch* CountMinSketch::clone() const
{
	CountMinSketch *s = new CountMinSketch(err_amount);
	s->sketch.merge(sketch);
	s->num_events = numEvents();
	return s;
}
