#include "CountMinSketch.h"
#include <string>
#include <cmath>
#include <algorithm>

CountMinSketch::CountMinSketch(float _err_amount, int _num_hh) : 
	num_events(0),
	err_amount(_err_amount),
	heavy_hitters(_num_hh, std::pair<uint32_t, uint32_t>(0,0))
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
	updateHeavyHitters(e, sketch.get(e_cstr, e_len));
}

int CountMinSketch::query(int e){
	std::string e_str = std::to_string(e);
	const char* e_cstr = e_str.c_str();
	size_t e_len = e_str.length();
	uint32_t count = sketch.get(e_cstr, e_len);
	updateHeavyHitters(e, count);
	return count;
}

unsigned CountMinSketch::numEvents() const {
	return num_events;
}

void CountMinSketch::merge(const CountMinSketch& o)
{
	num_events = o.numEvents() + numEvents();

	heavy_hitters.reserve(o.heavy_hitters.size() + heavy_hitters.size()); // preallocate memory
	heavy_hitters.insert(heavy_hitters.end(), o.heavy_hitters.begin(), o.heavy_hitters.end());
	
	auto hh_pred = [](std::pair<uint32_t, uint32_t> hh0, std::pair<uint32_t, uint32_t> hh1) {
		return hh0.second > hh1.second;
	};
	
	std::sort(heavy_hitters.begin(), heavy_hitters.end(), hh_pred);
	heavy_hitters.erase(heavy_hitters.begin() + o.heavy_hitters.size(), heavy_hitters.end());

	sketch.merge(o.sketch);
}

void CountMinSketch::filter(sketchFilter sf)
{
	num_events /= 2;
	sketch.filter(sf);
}

void CountMinSketch::clear()
{
	heavy_hitters = std::vector<std::pair<uint32_t, uint32_t>>(heavy_hitters.size(), std::pair<uint32_t, uint32_t>(0, 0));
	sketch.clear();
	num_events = 0;
}

CountMinSketch* CountMinSketch::clone() const
{
	CountMinSketch *s = new CountMinSketch(err_amount, heavy_hitters.size());
	s->sketch.merge(sketch);
	return s;
}

CountMinSketch* CountMinSketch::split(sketchFilter filter)
{
	auto hhs = heavy_hitters;
	clear();
	CountMinSketch* o = new CountMinSketch(err_amount, heavy_hitters.size());
	int n0 = 0;
	int n1 = 0;
	for (uint32_t i = 0; i < hhs.size(); i++) {
		auto hh = hhs[i];
		int select = filter(hh.first);
		if (select == 0) {
			o->num_events += hh.second;
			o->heavy_hitters[n0] = hh;
			n0++;
		}
		else {
			num_events += hh.second;
			heavy_hitters[n1] = hh;
			n1++;
		}
	}
	return o;
}

void CountMinSketch::updateHeavyHitters(uint32_t e, uint32_t count)
{
	uint32_t min = UINT32_MAX;
	uint32_t min_i = 0;
	for (uint32_t i = 0; i < heavy_hitters.size(); i++) {
		if (heavy_hitters[i].first == e) {
			heavy_hitters[i].second = count;
			return;
		}
		if (heavy_hitters[i].second < min) {
			min = heavy_hitters[i].second;
			min_i = i;
		}
	}
	if (count > min) {
		heavy_hitters[min_i].first = e;
		heavy_hitters[min_i].second = count;
	}
}
