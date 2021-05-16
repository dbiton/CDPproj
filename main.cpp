// SketchRing.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <map>
#include <random>

#include "RingSketch.h"

#define THREAD_NUM 4
#define ITER_NUM 16384*4
#define ITER_THREAD ITER_NUM/THREAD_NUM
#define KEY_MIN 0
#define KEY_MAX 1000
#define RING_SIZE_MIN 4
#define RING_SIZE_INIT 8
#define RING_SIZE_MAX 16
#define RING_MODIFY_SIZE_EXP 64*16 // Expectancy of number of iterations between expand or shrink operations
#define ERROR_MARGIN 0.5f

RingSketch sketch_ring(ERROR_MARGIN, RING_SIZE_INIT);
CountMinSketch sketch_regular(ERROR_MARGIN);
std::map<int, int> hashtable;
std::mutex mutex_hash;

enum ActionEnum{
	ADD,
	QUERY,
	ACTION_NUM
};

int randRange(int min, int max) {
	float n = ((float)rand()) / RAND_MAX;
	return std::round(n * (max - min) + min);
}

void run_thread() {
	float avg_err_rglr = 0.f, avg_err_ring = 0.f;
	int counter = 0;
	for (int iter = 0; iter < ITER_THREAD; iter++) {
		int action = randRange(0, ActionEnum::ACTION_NUM);
		int key = randRange(KEY_MIN, KEY_MAX);
		int expand = randRange(0, RING_MODIFY_SIZE_EXP);
		int shrink = randRange(0, RING_MODIFY_SIZE_EXP);

		if (expand && !shrink && sketch_ring.numSketchs() > RING_SIZE_MIN) {
			sketch_ring.shrink();
		}
		else if (!expand && shrink && sketch_ring.numSketchs() < RING_SIZE_MAX) {
			sketch_ring.expand();
		}

		switch (action)
		{
		case ActionEnum::ADD:
		{
			sketch_regular.add(key);
			sketch_ring.add(key);
			mutex_hash.lock();
			if (hashtable.find(key) != hashtable.end()) {
				hashtable[key]++;
			}
			else {
				hashtable[key] = 1;
			}
			mutex_hash.unlock();
			break;
		}
		case ActionEnum::QUERY:
		{
			float query_rglr = sketch_regular.query(key);
			float query_ring = sketch_ring.query(key);
			float query_hash = hashtable[key];
			
			float err_rglr = std::abs(query_rglr - query_hash);
			float err_ring = std::abs(query_ring - query_hash);
			avg_err_rglr += err_rglr;
			avg_err_ring += err_ring;
			counter++;
			if (err_rglr || err_ring) {
				std::cout << "[" << iter << "/" << ITER_THREAD <<"] QUERY:" << key << std::endl <<
					"[RING] RES:" << query_ring << " ERR:" << err_ring << " SIZE:" << sketch_ring.numSketchs() << std::endl <<
					"[RGLR] RES:" << query_rglr << " ERR:" << err_rglr << std::endl << std::endl;
			}
			break;
		}
		}
	}
	std::cout << std::endl <<
		"[RING] ERR AVG:" << avg_err_ring / counter << std::endl <<
		"[RGLR] ERR AVG:" << avg_err_rglr / counter << std::endl;
}

int main()
{
	run_thread();
	/*std::vector<std::thread> threads(THREAD_NUM);
	
	for (int tid = 0; tid < THREAD_NUM; tid++) {
		threads[tid] = std::thread(run_thread);
	}
	for (int tid = 0; tid < THREAD_NUM; tid++) {
		threads[tid].join();
	}*/
	return 0;
}1```