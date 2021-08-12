#include "Tester.h"
#include "RingSketch.h"

#include <random>
#include <algorithm>
#include <thread>
#include <chrono>

TesterResults Tester::test(const TesterSettings& _ts)
{
	tr.error_rglr = tr.error_ring = tr.t_query_rglr = tr.t_query_ring = tr.t_add_rglr = tr.t_add_ring = tr.t_expand = tr.t_shrink = 0;
	ts = _ts;
	sketch_ring = new RingSketch(ts.error_margin, ts.ring_size_init, ts.num_heavy_hitters);
	sketch_regular = new CountMinSketch(ts.error_margin, ts.num_heavy_hitters);
	hashtable = new std::map<int, int>();

	std::vector<std::thread> threads(ts.thread_num);
	for (int tid = 0; tid < ts.thread_num; tid++) {
		threads[tid] = std::thread([this] {run_thread(); });
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

void Tester::run_thread() {
	std::random_device rd;
	std::mt19937 mt(rd());
	std::binomial_distribution<> ud(ts.key_max, 0.5);
	// std::uniform_int_distribution<> ud(ts.key_min, ts.key_max);

	int counter_modify_size = ts.ring_add_per_modify_size;
	int count_add = 0;
	float avg_err_rglr = 0.f, avg_err_ring = 0.f;
	int counter = 0;

	std::chrono::high_resolution_clock::time_point start, end;
	std::chrono::microseconds duration;
	uint64_t ms_query_rglr=0, ms_query_ring=0, ms_add_rglr=0, ms_add_ring=0, ms_expand=0, ms_shrink=0;

	for (int iter = 0; iter < ts.iter_num / ts.thread_num; iter++) {
		int key = ud(mt);
		if (counter_modify_size-- == 0) {
			counter_modify_size = ts.ring_add_per_modify_size;
			if (rand() % 2 == 0) {
				if (sketch_ring->numSketchs() > ts.ring_size_min) {
					start = std::chrono::high_resolution_clock::now();
					sketch_ring->shrink();
					end = std::chrono::high_resolution_clock::now();
					duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
					ms_shrink += duration.count();
				}
				else {
					start = std::chrono::high_resolution_clock::now();
					sketch_ring->expand();
					end = std::chrono::high_resolution_clock::now();
					duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
					ms_expand += duration.count();
				}
			}
			else {
				if (sketch_ring->numSketchs() < ts.ring_size_max) {
					start = std::chrono::high_resolution_clock::now();
					sketch_ring->expand();
					end = std::chrono::high_resolution_clock::now();
					duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
					ms_expand += duration.count();
				}
				else {
					start = std::chrono::high_resolution_clock::now();
					sketch_ring->shrink();
					end = std::chrono::high_resolution_clock::now();
					duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
					ms_shrink += duration.count();
				}
			}
		}

		// ADD
		count_add++;
		start = std::chrono::high_resolution_clock::now();
		sketch_regular->add(key);
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		ms_add_rglr += duration.count();

		start = std::chrono::high_resolution_clock::now();
		sketch_ring->add(key);
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		ms_add_ring += duration.count();

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
		start = std::chrono::high_resolution_clock::now();
		int query_rglr = sketch_regular->query(key);
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		ms_query_rglr += duration.count();

		start = std::chrono::high_resolution_clock::now();
		int query_ring = sketch_ring->query(key);
		end = std::chrono::high_resolution_clock::now();
		duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
		ms_query_ring += duration.count();

		float err_rglr = std::abs(query_rglr - query_hash);
		float err_ring = std::abs(query_ring - query_hash);
		avg_err_rglr += err_rglr;
		avg_err_ring += err_ring;

		counter++;
	}
	mutex.lock();
	tr.error_rglr += avg_err_rglr / counter;
	tr.error_ring += avg_err_ring / counter;
	tr.t_add_rglr += (double)ms_add_rglr / 1000000 / counter;
	tr.t_add_ring += (double)ms_add_ring / 1000000 / counter;
	tr.t_query_rglr += (double)ms_query_rglr / 1000000 / counter;
	tr.t_query_ring += (double)ms_query_ring / 1000000 / counter;
	tr.t_shrink += (double)ms_query_ring / 1000000;
	tr.t_expand += (double)ms_query_ring / 1000000;
	mutex.unlock();
}