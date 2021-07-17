#include <thread>
#include "Tester.h"
#include "RingSketch.h"
#include <random>
#include <algorithm>

TesterResults Tester::test(const TesterSettings& _ts)
{
	tr.error_rglr = 0;
	tr.error_ring = 0;
	ts = _ts;
    sketch_ring = new RingSketch(ts.error_margin, ts.ring_size_init, ts.num_heavy_hitters);
    sketch_regular = new CountMinSketch(ts.error_margin, ts.num_heavy_hitters);
	hashtable = new std::map<int, int>();

	std::vector<std::thread> threads(ts.thread_num);
	for (int tid = 0; tid < ts.thread_num; tid++) {
		threads[tid] = std::thread([this] {run_thread(); } );
	}
	for (int tid = 0; tid < ts.thread_num; tid++) {
		threads[tid].join();
	}

	tr.error_rglr /= ts.thread_num;
	tr.error_ring /= ts.thread_num;

	delete sketch_ring;
	delete sketch_regular;
	delete hashtable;
	
	return tr;
}

enum ActionEnum {
	ADD,
	QUERY,
	ACTION_NUM
};

#include <iostream>

void show_distrib() {
	std::random_device rd;
	std::mt19937 mt(rd());
	std::binomial_distribution<> bd(8, 0.5);
	std::uniform_int_distribution<> ud(0, 8);

	std::map<int, int> hist;
	std::cout << "BINOMIAL:" << std::endl;
	for (int n = 0; n < 10000; ++n) {
		++hist[bd(mt)];
	}
	for (auto p : hist) {
		std::cout << p.first <<
			' ' << std::string(p.second / 100, '*') << '\n';
	}

	std::cout << "UNIFORM:" << std::endl;
	hist.clear();
	for (int n = 0; n < 10000; ++n) {
		++hist[ud(mt)];
	}
	for (auto p : hist) {
		std::cout << p.first <<
			' ' << std::string(p.second / 100, '+') << '\n';
	}
}

void Tester::run_thread() {
	std::random_device rd;
	std::mt19937 mt(rd());
	std::binomial_distribution<> ud(ts.key_max, 0.5);
	// std::uniform_int_distribution<> ud(ts.key_min, ts.key_max);

	int counter_modify_size = ts.ring_add_per_modify_size;
	int count_add = 0;
	float avg_err_rglr = 0.f, avg_err_ring = 0.f;
	int counter = 0;
	for (int iter = 0; iter < ts.iter_num / ts.thread_num; iter++) {
		int key = ud(mt);
		if (counter_modify_size-- == 0) {
			counter_modify_size = ts.ring_add_per_modify_size;
			if (rand() % 2 == 0) {
				if (sketch_ring->numSketchs() > ts.ring_size_min) {
					sketch_ring->shrink();
				}
				else {
					sketch_ring->expand();
				}
			}
			else {
				if (sketch_ring->numSketchs() < ts.ring_size_max) {
					sketch_ring->expand();
				}
				else {
					sketch_ring->shrink();
				}
			}
		}

		// ADD
		count_add++;
		sketch_regular->add(key);
		sketch_ring->add(key);
		int query_hash;
		if (hashtable->find(key) != hashtable->end()) {
			query_hash = (*hashtable)[key];
			(*hashtable)[key] = query_hash + 1;
			query_hash++;
		}
		else {
			(*hashtable)[key] = 1;
			query_hash = 1;
		}

		// QUERY
		int query_rglr = sketch_regular->query(key);
		int query_ring = sketch_ring->query(key);

		float err_rglr = std::abs(query_rglr - query_hash);
		float err_ring = std::abs(query_ring - query_hash);
		avg_err_rglr += err_rglr;
		avg_err_ring += err_ring;

		counter++;
	}
	mutex.lock();
	tr.error_rglr += avg_err_rglr / counter;
	tr.error_ring += avg_err_ring / counter;
	mutex.unlock();
}