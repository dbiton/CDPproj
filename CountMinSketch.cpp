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
	
	// merge heavy hitters - we want to keep the most heavy from both sketchs
	heavy_hitters.reserve(o.heavy_hitters.size() + heavy_hitters.size());
	heavy_hitters.insert(heavy_hitters.end(), o.heavy_hitters.begin(), o.heavy_hitters.end());
	
	// used for sorting heavy hitters
	auto hh_pred = [](std::pair<uint32_t, uint32_t> hh0, std::pair<uint32_t, uint32_t> hh1) {
		return hh0.second > hh1.second;
	};
	
	std::sort(heavy_hitters.begin(), heavy_hitters.end(), hh_pred);
	// get rid of the least heavy hitters
	heavy_hitters.erase(heavy_hitters.begin() + o.heavy_hitters.size(), heavy_hitters.end());

	sketch.merge(o.sketch);
}

void CountMinSketch::clear()
{
	// set heavy hitters to an empty vector
	heavy_hitters = std::vector<std::pair<uint32_t, uint32_t>>(heavy_hitters.size(), std::pair<uint32_t, uint32_t>(0, 0));
	sketch.clear();
	num_events = 0;
}

CountMinSketch* CountMinSketch::split(sketchFilter filter)
{
	// save heavy hitters so they won't get erased when we clear this sketch
	auto hhs = heavy_hitters;
	clear();
	CountMinSketch* o = new CountMinSketch(err_amount, heavy_hitters.size());
	// init indexes for hhs in both sketchs
	int n0 = 0;
	int n1 = 0;
	for (uint32_t i = 0; i < hhs.size(); i++) {
		auto hh = hhs[i];
		// filter is given by the user, we keep only values the filter returns a non zero value for
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
		// if it already is a heavy hitter, we just need to update it's count and leave
		if (heavy_hitters[i].first == e) {
			heavy_hitters[i].second = count;
			return;
		}
		// also get heavy_hitter's minimum while you're at it
		if (heavy_hitters[i].second < min) {
			min = heavy_hitters[i].second;
			min_i = i;
		}
	}
	// if this value's count is larger than our smallest heavy hitter, we can replace it
	if (count > min) {
		heavy_hitters[min_i].first = e;
		heavy_hitters[min_i].second = count;
	}
}
