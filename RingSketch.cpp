#include "RingSketch.h"

#include <cassert>

const uint64_t KEY_DEFAULT = 0xDEAD4263BABE9523;
uint64_t key_global = KEY_DEFAULT;

RingSketch::RingSketch(float err_amount_initial, int num_sketch_initial) :
	error_amount(err_amount_initial),
	redir_graph(num_sketch_initial),
	size_initial(num_sketch_initial),
	size_curr(num_sketch_initial),
	total_added(0.f)
{
	for (int i = 0; i < num_sketch_initial; i++) {
		sketchs.push_back(new CountMinSketch(num_sketch_initial * error_amount));
		sketchs_mutexes.push_back(new std::mutex());
	}
}

void RingSketch::add(uint32_t e)
{
	uint32_t i = getSketchIdx(e);
	sketchs_mutexes[i]->lock();
	sketchs[i]->add(e);
	total_added+=1.f;
	sketchs_mutexes[i]->unlock();
}

float RingSketch::query(uint32_t e)
{
	uint32_t i = getSketchIdx(e);
	sketchs_mutexes[i]->lock();
	float n = sketchs[i]->query(e);
	n = std::min(total_added, n);
	sketchs_mutexes[i]->unlock();
	return n;
}

void RingSketch::expand()
{
	lockAll();
	error_amount *= numSketchs() / (numSketchs() + 1);
	int i = getFullestSketchIdx();
	CountMinSketch* sketch_split = sketchs[i];
	
	key_global = redir_graph.getKey(i);
	auto filter_split = [](madoka_uint64 v) {
		bool select_original = hash(v, key_global, 2);
		if (select_original) {
			return v;
		}
		else return (madoka_uint64)0;
	};
	auto filter_new = [](madoka_uint64 v) {
		bool select_original = hash(v, key_global, 2);
		if (!select_original) {
			return v;
		}
		else return (madoka_uint64)0;
	};

	
	CountMinSketch* sketch_new = sketch_split->clone();
	
	sketch_new->filter(filter_new);
	sketch_split->filter(filter_split);
	sketchs[i] = nullptr;

	sketchs.push_back(sketch_new);
	sketchs.push_back(sketch_split);
	sketchs_mutexes.push_back(new std::mutex());
	sketchs_mutexes.back()->lock();
	sketchs_mutexes.push_back(new std::mutex());
	redir_graph.split(i);
	sketchs_mutexes.back()->lock();
	size_curr++;
	unlockAll();
}

void RingSketch::shrink()
{
	lockAll();
	if (size_curr > 1) {
		int i = getEmptiestSketchIdx();
		CountMinSketch* sketch_removed = sketchs[i];
		int i_successor = (i + 1) % sketchs.size();
		while (!sketchs[i_successor]) {
			i_successor = (i_successor + 1) % sketchs.size();
		}
		CountMinSketch* sketch_successor = sketchs[i_successor];
		sketch_successor->merge(*sketch_removed);
		error_amount *= 2;
		redir_graph.redirect(i, i_successor);
		delete sketch_removed;
		sketchs[i] = nullptr;
		size_curr--;
	}
	unlockAll();
}

unsigned RingSketch::getFullestSketchIdx()
{
	unsigned max_n = 0;
	unsigned max_i = 0;
	for (unsigned i = 0; i < sketchs.size(); i++) {
		if (sketchs[i]) {
			int cur_n = sketchs[i]->numEvents();
			if (cur_n > max_n) {
				max_n = cur_n;
				max_i = i;
			}
		}
	}
	return max_i;
}

unsigned RingSketch::getEmptiestSketchIdx()
{
	unsigned min_n = -1; // init to MAX_UNSIGNED
	unsigned min_i = 0;
	for (unsigned i = 0; i < sketchs.size(); i++) {
		if (sketchs[i]) {
			int cur_n = sketchs[i]->numEvents();
			if (cur_n < min_n) {
				min_n = cur_n;
				min_i = i;
			}
		}
	}
	return min_i;
}

int RingSketch::numSketchs()
{
	return size_curr;
}

void RingSketch::lockAll()
{
	for (auto& mutex : sketchs_mutexes) {
		mutex->lock();
	}
}

void RingSketch::unlockAll()
{
	for (auto& mutex : sketchs_mutexes) {
		mutex->unlock();
	}
}

uint32_t hash(uint32_t v, uint64_t key, uint32_t num_buckets) {
	uint64_t p = (1ULL << 61) - 1;	// 2^61-1 is prime
	uint32_t n = (uint32_t) key;
	uint32_t m = (uint32_t)(key >> 32);
	return ((uint64_t)v * n + m) % p % num_buckets;
}

uint32_t RingSketch::getSketchIdx(uint32_t e)
{
	uint32_t path_length = 0;
	uint32_t sketch_idx_initial = hash(e, KEY_DEFAULT, size_initial);
	uint32_t sketch_idx;
	for (sketch_idx = sketch_idx_initial; !redir_graph.isRoot(sketch_idx); path_length++) {
		if (redir_graph.isSplit(sketch_idx)) {
			bool select_original = hash(e, redir_graph.getKey(sketch_idx), 2);
			sketch_idx = redir_graph.getChild(sketch_idx, !select_original);
		}
		else {
			sketch_idx = redir_graph.getRoot(sketch_idx);
		}
	}

	assert(sketchs[sketch_idx]);

	return sketch_idx;
}