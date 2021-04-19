// SketchRing.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <map>
#include <random>

#include "RingSketch.h"

#define ITER_NUM 4096
#define KEY_MIN -4096
#define KEY_MAX  4096

enum ActionEnum{
	ADD,
	QUERY,
	COUNT,
	ACTION_NUM
};

int randRange(int min, int max) {
	double n = ((double)rand()) / RAND_MAX;
	return min + (max - min) * n;
}

int main()
{
	RingSketch sketch_ring(8, 0.5f);
	CountMinSketch sketch_regular(0.5f, 0.5f);
	std::map<int, int> hashtable;
	for (int iter = 0; iter < ITER_NUM; iter++) {
		int action = randRange(0, ActionEnum::ACTION_NUM);
		int key = randRange(KEY_MIN, KEY_MAX);
		switch (action)
		{
		case ActionEnum::ADD:
		{
			sketch_regular.add(key);
			sketch_ring.add(key);
			hashtable[key]++;
			std::cout << "[ADD] " << key << std::endl;
			break;
		}
		case ActionEnum::QUERY:
		{
			int query_rglr = sketch_regular.query(key);
			int query_ring = sketch_ring.query(key);
			int query_hash = hashtable[key];
			std::cout << "[QUERY] RING:" << query_ring << " REGULAR:" << query_rglr << " HASH:" << query_hash << std::endl;
			break;
		}
		case ActionEnum::COUNT:
			break;
		default:
			break;
		}
	}
	return 0;
}