#pragma once

#include <cstdint>
#include <vector>


class RedirGraph
{
	struct RedirNode {
		uint32_t child0, child1;
		uint64_t key;

		RedirNode();
	};

	std::vector<RedirNode> nodes;
	uint32_t size_initial;
public:
	RedirGraph(uint32_t size_initial);

	uint32_t size() const;
	uint32_t allocNode(); // allocate a new node, return it's index

	void redirect(uint32_t i_src, uint32_t i_dst);	
	void split(uint32_t i, uint32_t& i_split0, uint32_t& i_split1);

	bool isRoot(uint32_t i) const;
	bool isSplit(uint32_t i) const;
	uint32_t getChild(uint32_t i, bool child1);
	uint32_t getRedir(uint32_t i) const;
	uint64_t getKey(uint32_t i);
	uint32_t getRoot(uint32_t i);

	void print();
private:
	static uint64_t randKey();
};