#pragma once

#include "CountMinSketch.h"

#include <vector>
#include <mutex>

class RingSketch {
	std::vector<CountMinSketch*> sketchs;
	std::vector<std::mutex*> sketchs_mutexes;
	float error_amount, error_prob;
public:
	RingSketch(float err_prob, float err_amount_initial, int num_sketch_initial = 1);

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