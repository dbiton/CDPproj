#include "RingSketch.h"
#include <cassert>
#include <algorithm>
#include <iostream>

const uint64_t KEY_DEFAULT = 0x6666DEADBABE3333;
uint64_t key_global = KEY_DEFAULT;

uint32_t hash(uint32_t v, uint64_t key, uint32_t num_buckets) {
	uint64_t p = (1ULL << 61) - 1;	// 2^61-1 is prime
	uint32_t n = (uint32_t)key;
	uint32_t m = (uint32_t)(key >> 32);
	return ((uint64_t)v * n + m) % p % num_buckets;
}

RingSketch::RingSketch(float err_amount_initial, float err_prob, int num_sketch_initial, uint32_t _heavy_hitters_num) :
	error_probability(err_prob),
	error_amount(err_amount_initial),
	redir_graph(num_sketch_initial),
	size_initial(num_sketch_initial),
	size_curr(num_sketch_initial),
	heavy_hitters_num(_heavy_hitters_num),
	total_added(0.f)
{
	for (int i = 0; i < num_sketch_initial; i++) {
		sketchs[i] = new CountMinSketch(num_sketch_initial * error_amount, err_prob);
		sketchs_mutexes[i] = new std::mutex();
	}
}

void RingSketch::printSizes() {
	std::cout << "XXXXXXXXXXXXXXXXX" << std::endl;
	for (auto& s : sketchs) {
		std::cout << s.second->numEvents() << " ";
	}
	std::cout << std::endl << "XXXXXXXXXXXXXXXXX" << std::endl;
}

void RingSketch::add(uint32_t e)
{
	uint32_t i = getSketchIdx(e);
	//sketchs_mutexes[i]->lock();
	sketchs[i]->add(e);
	total_added += 1.f;
	//sketchs_mutexes[i]->unlock();
}

void RingSketch::add(std::set<uint32_t> es)
{
	if (es.size() == 1) {
		add(*es.begin());
	}
	else {
		add(hashSet(es));
		std::set<uint32_t> es_copy = es;
		for (auto& e : es) {
			es_copy.erase(e);
			add(es_copy);
			es_copy.insert(e);
		}
	}
}

float RingSketch::query(uint32_t e)
{
	uint32_t i = getSketchIdx(e);
	//sketchs_mutexes[i]->lock();
	float n = sketchs[i]->query(e);
	n = std::min(total_added, n);
	//sketchs_mutexes[i]->unlock();
	return n;
}

float RingSketch::query(const std::set<uint32_t>& es)
{
	return query(hashSet(es));
}

void RingSketch::expand()
{
	error_amount *= numSketchs() / (numSketchs() + 1);
	int i = getFullestSketchIdx();

	key_global = redir_graph.getKey(i);

	CountMinSketch* sketch0 = sketchs[i];
	CountMinSketch* sketch1 = sketch0;

	std::mutex* mutex0 = sketchs_mutexes[i];
	std::mutex* mutex1 = new std::mutex;

	//sketch0->clear();
	//sketch1->clear();

	sketchs.erase(i);
	sketchs_mutexes.erase(i);

	uint32_t i0, i1;
	redir_graph.split(i, i0, i1);

	sketchs[i0] = sketch0;
	sketchs[i1] = sketch1;

	sketchs_mutexes[i0] = mutex0;
	sketchs_mutexes[i1] = mutex1;

	size_curr++;
}

void RingSketch::shrink()
{
	if (size_curr > 1) {
		uint32_t i_2nd_emptiest;
		uint32_t i_emptiest = getEmptiestSketchIdx(i_2nd_emptiest);
		
		CountMinSketch* sketch_emptiest = sketchs[i_emptiest];
		CountMinSketch* sketch_2nd_emptiest = sketchs[i_2nd_emptiest];
		
		sketch_emptiest->merge(*sketch_2nd_emptiest);
		error_amount *= 2;
		redir_graph.redirect(i_emptiest, i_2nd_emptiest);
		
		delete sketch_emptiest;
		std::mutex*  mutex_erased = sketchs_mutexes[i_emptiest];
		
		sketchs.erase(i_emptiest);
		sketchs_mutexes.erase(i_emptiest);
		
		delete mutex_erased;

		size_curr--;
	}

}

uint32_t RingSketch::getFullestSketchIdx()
{
	uint32_t max_n = 0;
	uint32_t max_i = 0;
	for (auto& idx_sketch_pair : sketchs) {
		uint32_t i = idx_sketch_pair.first;
		CountMinSketch* sketch = idx_sketch_pair.second;
		int cur_n = sketch->numEvents();
		if (cur_n > max_n) {
			max_n = cur_n;
			max_i = i;
		}
	}
	return max_i;
}

uint32_t RingSketch::getEmptiestSketchIdx(uint32_t& second_emptiest)
{
	auto& it = sketchs.begin();
	uint32_t i0, n0, i1, n1;
	i0 = it->first;
	n0 = it->second->numEvents();
	it++;
	i1 = it->first;
	n1 = it->second->numEvents();

	uint32_t min_i0, min_n0, min_i1, min_n1;

	if (n0 < n1) {
		min_i0 = i0;
		min_n0 = n0;
		min_i1 = i1;
		min_n1 = n1;
	}
	else {
		min_i0 = i1;
		min_n0 = n1;
		min_i1 = i0;
		min_n1 = n0;
	}

	it++;

	while (it != sketchs.end()) {
		
		uint32_t i = it->first;
		CountMinSketch* sketch = it->second;
		int cur_n = sketch->numEvents();
		if (cur_n < min_n0) {
			min_n1 = min_n0;
			min_n0 = cur_n;
			min_i1 = min_i0;
			min_i0 = i;
		}
		else if (cur_n < min_n1 && cur_n != min_n0) {
			min_n1 = cur_n;
			min_i1 = i;
		}

		it++;
	}

	second_emptiest = min_i1;
	return min_i0;
}

int RingSketch::numSketchs()
{
	return size_curr;
}

void RingSketch::lockAll()
{
	for (auto& mutex_key_pair : sketchs_mutexes) {
		//mutex_key_pair.second->lock();
	}
}

void RingSketch::unlockAll()
{
	for (auto& mutex_key_pair : sketchs_mutexes) {
		//mutex_key_pair.second->unlock();
	}
}

uint32_t RingSketch::getSketchIdx(uint32_t e)
{
	uint32_t sketch_idx_initial = hash(e, KEY_DEFAULT, size_initial);
	uint32_t sketch_idx;
	for (sketch_idx = sketch_idx_initial; !redir_graph.isRoot(sketch_idx);) {
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

uint32_t RingSketch::hashSet(const std::set<uint32_t>& es)
{
	std::string es_str;
	for (auto& e : es) {
		es_str += std::to_string(e) + "|";
	}
	std::cout << es_str << std::endl;
	return std::hash<std::string>{}(es_str);
}

void RingSketch::targetResizeCheck()
{
	if (!has_target_err) {
		return;
	}

	// expand
	error_amount *= numSketchs() / (numSketchs() + 1);
	// shrink	
	error_amount *= 2;
}