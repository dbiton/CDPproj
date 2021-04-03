#pragma once

#include "CountMinSketch.h"

#include <vector>
#include <mutex>

class RingSketch {
	std::vector<CountMinSketch*> sketchs;
	std::vector<std::mutex> sketchs_mutexes;
	float error;
public:
	RingSketch(int num_sketch_initial, float err_initial);

	void add(int e);
	float query(int e);

	void expand();
	void shrink();
private:
	int hash(int e);
	unsigned getFullestSketchIdx();
	unsigned getEmptiestSketchIdx();
	int numSketchs();
	void lockAll();
	void unlockAll();
};