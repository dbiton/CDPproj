#pragma once


#include <map>
#include <vector>
#include "madoka/lib/madoka.h"


typedef madoka::SketchFilter sketchFilter;

class CountMinSketch {
	std::vector<std::pair<uint32_t, uint32_t>> heavy_hitters;
	madoka::Sketch sketch;
	unsigned num_events;
	float err_amount;
public:
	CountMinSketch(float err_amount, int num_hh);
	~CountMinSketch();

	void add(int e);
	int query(int e);

	unsigned numEvents() const;
	
	void merge(const CountMinSketch& sketch);	
	void clear();
	CountMinSketch* split(sketchFilter filter);
private:
	void updateHeavyHitters(uint32_t e, uint32_t count);
};