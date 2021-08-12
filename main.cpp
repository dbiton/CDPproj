#include <iostream>

#include "RingSketch.h"
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
	ts.ring_add_per_modify_size = 256;
	ts.num_heavy_hitters = 64;
	ts.error_margin = 0.1;
	for (int k = 6; k <= 10; k++) {
		for (int i = 10; i <= 19; i++) {
			ts.ring_add_per_modify_size = std::pow(2, k);
			ts.iter_num = std::pow(2, i);
			TesterResults tr = tester.test(ts);
			std::cout << ts.ring_add_per_modify_size << " " << ts.iter_num;
			std::cout << " " << tr.t_add_rglr << " " << tr.t_add_ring << std::endl;
		}
	}
	return 0;
}