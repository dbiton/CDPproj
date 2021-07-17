#include <iostream>
#include <map>
#include <random>

#include "RingSketch.h"
#include "Logger.h"

#define ITER_NUM 4096*4
#define THREAD_NUM 1
#define ITER_THREAD ITER_NUM/THREAD_NUM
#define KEY_MIN 1
#define KEY_MAX 128
#define RING_SIZE_MIN 2
#define RING_SIZE_INIT 4
#define RING_SIZE_MAX 8
#define RING_MODIFY_SIZE_EXP 128 // Expectancy of number of iterations between expand or shrink operations
#define ERROR_MARGIN 0.05f
#define NUM_HH 32


Logger logger;

RingSketch sketch_ring(ERROR_MARGIN, RING_SIZE_INIT, NUM_HH);
CountMinSketch sketch_regular(ERROR_MARGIN, NUM_HH);
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
	int count_add = 0;
	float avg_err_rglr = 0.f, avg_err_ring = 0.f;
	int counter = 0;
	for (int iter = 0; iter < ITER_THREAD; iter++) {
		int key = randRange(KEY_MIN, KEY_MAX);
		int expand = randRange(0, RING_MODIFY_SIZE_EXP);
		int shrink = randRange(0, RING_MODIFY_SIZE_EXP);

		if (expand && !shrink && sketch_ring.numSketchs() > RING_SIZE_MIN) {
			sketch_ring.shrink();
		}
		else if (!expand && shrink && sketch_ring.numSketchs() < RING_SIZE_MAX) {
			sketch_ring.expand();
		}

		// ADD
		count_add++;
		sketch_regular.add(key);
		sketch_ring.add(key);
		mutex_hash.lock();
		if (hashtable.find(key) != hashtable.end()) {
			hashtable[key] = hashtable[key]+1;
		}
		else {
			hashtable[key] = 1;
		}
		mutex_hash.unlock();

		// QUERY
		float query_rglr = sketch_regular.query(key);
		float query_ring = sketch_ring.query(key);
		float query_hash;
		if (hashtable.find(key) != hashtable.end()) {
			query_hash = hashtable[key];
		}
		else {
			query_hash = 0;
		}

		sketch_ring.printSizes();

		float err_rglr = std::abs(query_rglr - query_hash);
		float err_ring = std::abs(query_ring - query_hash);
		avg_err_rglr += err_rglr;
		avg_err_ring += err_ring;
		counter++;

		std::cout << "ACT:" << query_hash << " RGLR:"<< query_rglr << " RING:" <<query_ring << std::endl;

		logger.log(count_add, sketch_ring.numSketchs(), sizeof(sketch_ring), query_rglr, query_hash, query_ring);
	}
	 std::cout << std::endl <<
		"[RING] ERR AVG:" << avg_err_ring / counter << std::endl <<
		"[RGLR] ERR AVG:" << avg_err_rglr / counter << std::endl;
	 sketch_ring.printSizes();
	logger.write("output_test.txt");
}

#include "Tester.h"

int main()
{
	Tester tester;
	TesterSettings ts;
	ts.iter_num = 0;
	ts.thread_num = 1;
	ts.key_min = 0;
	ts.key_max = 4096;
	ts.ring_size_min = 2;
	ts.ring_size_max = 8;
	ts.ring_size_init = 4;
	ts.ring_add_per_modify_size = 16;
	ts.num_heavy_hitters = 64;
	ts.error_margin = 0.1;
	for (int k = 6; k <= 10; k++) {
		for (int i = 10; i <= 19; i++) {
			ts.ring_add_per_modify_size = std::pow(2, k);
			ts.iter_num = std::pow(2, i);
			TesterResults tr = tester.test(ts);
			std::cout << ts.ring_add_per_modify_size << " " << ts.iter_num;
			std::cout << " " << tr.error_rglr << " " << tr.error_ring << std::endl;
		}
	}
	return 0;
}