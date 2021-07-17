#pragma once

/*
	WRAPPER

	This is used in order to decouple madoka from our code. Any Count Min Sketch that implements this 
	interface can be used with RingSketch.

	Note: for now, sketchs only work with numbers. We can
	trivially use templates in order to generalize the data 
	structure.
*/

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
	// filter should remove about half of the events from the sketch for num_events to be updated accuretly 
	void filter(sketchFilter filter);
	
	void clear();

	CountMinSketch* clone() const;
	CountMinSketch* split(sketchFilter filter);
private:
	void updateHeavyHitters(uint32_t e, uint32_t count);
};