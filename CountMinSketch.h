#pragma once

// Interface

/*
	Note: for now, sketchs only work with numbers. We can
	trivially use templates in orde to generalize the data 
	structure.
*/

#include <vector>

class CountMinSketch {
public:
	CountMinSketch(float err);

	void add(int e);
	void remove(int e);
	float query(int e);
	int numEvents();
	// returns all events held in sketch
	std::vector<int> collectAll();
	// returns only the "heavy hitter" events held in sketch
	std::vector<int> collectHeavyHitters();
};