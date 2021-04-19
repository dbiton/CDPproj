#include "RingSketch.h"

RingSketch::RingSketch(float err_prob_initial, float err_amount_initial, int num_sketch_initial):
	error_amount(err_amount_initial),
	error_prob(err_prob_initial)
{
	for (int i = 0; i < num_sketch_initial; i++) {
		sketchs.push_back(new CountMinSketch(error_prob, num_sketch_initial * error_amount));
		sketchs_mutexes.push_back(new std::mutex());
	}
}

void RingSketch::add(int e)
{
	int i = hash(e);
	sketchs_mutexes[i]->lock();
	sketchs[i]->add(e);
	sketchs_mutexes[i]->unlock();
}

float RingSketch::query(int e)
{
	int i = hash(e);
	sketchs_mutexes[i]->lock();
	float n = sketchs[i]->query(e);
	sketchs_mutexes[i]->unlock();
	return n;
}

void RingSketch::expand()
{
	lockAll();
	error_amount *= numSketchs() / (numSketchs() + 1);
	int j = getFullestSketchIdx();
	CountMinSketch* sketch_j = sketchs[j];
	int i = (j - 1) % numSketchs();
	if (i < 0) i += numSketchs(); // negative index when j = 0 fix
	CountMinSketch* sketch_i = sketchs[i];
	CountMinSketch* sketch_new = new CountMinSketch(error_prob, numSketchs() * error_amount);
	sketchs.insert(sketchs.begin() + i, sketch_new);
	std::vector<int> events_split = sketch_i->collectAll();
	for (auto& e : events_split) {
		/* int e_index = hash(e);
		if () {
			sketch_split->remove(e);
			sketch_new->add(e);
		} */
	}
	unlockAll();
}

void RingSketch::shrink()
{
	lockAll();
	int i = getEmptiestSketchIdx();
	CountMinSketch* sketch_removed = sketchs[i];
	int i_successor = (i + 1) % numSketchs();
	CountMinSketch* sketch_successor = sketchs[i_successor];
	std::vector<int> heavy_hitters = sketch_removed->collectHeavyHitters();
	sketchs.erase(sketchs.begin() + i_successor);
	sketchs_mutexes.erase(sketchs_mutexes.begin() + i_successor);
	delete sketch_removed;
	for (auto& e : heavy_hitters) {
		sketch_successor->add(e);
	}
	error_amount *= 2;
	unlockAll();
}

unsigned RingSketch::getFullestSketchIdx()
{
	unsigned max_n = 0;
	unsigned max_i = 0;
	for (unsigned i = 0; i < numSketchs(); i++) {
		int cur_n = sketchs[i]->numEvents();
		if (cur_n > max_n) {
			max_n = cur_n;
			max_i = i;
		}
	}
	return max_i;
}

unsigned RingSketch::getEmptiestSketchIdx()
{
	unsigned min_n = -1; // init to MAX_UNSIGNED
	unsigned min_i = 0;
	for (unsigned i = 0; i < numSketchs(); i++) {
		int cur_n = sketchs[i]->numEvents();
		if (cur_n < min_n) {
			min_n = cur_n;
			min_i = i;
		}
	}
	return min_i;
}

int RingSketch::numSketchs()
{
	return sketchs.size();
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

int RingSketch::hash(int e)
{
	return e % numSketchs();
}
