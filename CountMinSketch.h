#pragma once

/*
	WRAPPER

	This is used in order to decouple madoka from our code. Any Count Min Sketch that implements this 
	interface can be used with RingSketch.

	Note: for now, sketchs only work with numbers. We can
	trivially use templates in order to generalize the data 
	structure.
*/

#include <vector>
#include "madoka/lib/madoka.h"

typedef madoka::SketchFilter sketchFilter;

class CountMinSketch {
	madoka::Sketch sketch;
	unsigned num_events;
	float flt_median;
	float err_amount;
public:
	CountMinSketch(float err_amount);
	~CountMinSketch();

	void add(int e);
	float query(int e) const;

	unsigned numEvents() const;
	unsigned median() const;
	
	void merge(const CountMinSketch& sketch);
	void filter(sketchFilter filter);
	
	CountMinSketch* clone() const;
};