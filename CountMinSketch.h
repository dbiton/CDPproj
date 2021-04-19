#pragma once

// Interface

/*
	Note: for now, sketchs only work with numbers. We can
	trivially use templates in order to generalize the data 
	structure.
*/

#include <vector>
#include "madoka/lib/madoka.h"

class CountMinSketch {
	madoka::Sketch sketch;
	unsigned num_events;
public:
	CountMinSketch(float err_prob, float err_amount);
	~CountMinSketch();

	void add(int e);
	float query(int e);
	unsigned numEvents();

	// returns all events held in sketch
	std::vector<int> collectAll();
	// returns only the "heavy hitter" events held in sketch
	std::vector<int> collectHeavyHitters();
};