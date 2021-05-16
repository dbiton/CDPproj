#include "RedirGraph.h"

#include <random>
#include <cassert>
#include <iostream>

RedirGraph::RedirGraph(uint32_t size_initial)
{
	for (uint32_t i = 0; i < size_initial; i++) {
		nodes.push_back(RedirNode{UINT32_MAX, UINT32_MAX, randKey()});
	}
}

uint32_t RedirGraph::size() const
{
	return nodes.size();
}

void RedirGraph::redirect(uint32_t i_src, uint32_t i_dst)
{
	assert(i_dst < size() && i_src < size() && isRoot(i_src));

	nodes[i_src].child0 = i_dst;
}

void RedirGraph::split(uint32_t i)
{
	assert(i < size() && isRoot(i));
	
	RedirNode& n = nodes[i];
	n.child0 = size();
	n.child1 = size()+1;
	
	nodes.push_back({ UINT32_MAX, UINT32_MAX, randKey() });
	nodes.push_back({ UINT32_MAX, UINT32_MAX, randKey() });
}

bool RedirGraph::isRoot(uint32_t i) const
{
	assert(i < size());

	return nodes[i].child0 == UINT32_MAX && nodes[i].child1 == UINT32_MAX;
}

bool RedirGraph::isSplit(uint32_t i) const
{
	assert(i < size());

	return nodes[i].child0 != UINT32_MAX && nodes[i].child1 != UINT32_MAX;
}

uint32_t RedirGraph::getChild(uint32_t i, bool child1)
{
	assert(isSplit(i));

	return child1 ? nodes[i].child1 : nodes[i].child0;
}

uint32_t RedirGraph::getRedir(uint32_t i) const
{
	assert(nodes[i].child0 != UINT32_MAX);
	
	if (nodes[i].child1 != UINT32_MAX) {
		std::cout << "redir error: " << nodes[i].child1 << std::endl;
	}

	return nodes[i].child0;
}

uint64_t RedirGraph::getKey(uint32_t i)
{
	return nodes[i].key;
}

uint32_t RedirGraph::getRoot(uint32_t idx)
{
	std::vector<uint32_t> indice;
	uint32_t i = idx;
	while (!isRoot(i) && !isSplit(i)) {
		indice.push_back(i);
		i = getRedir(i);
	}
	for (auto& j : indice) {
		nodes[j].child0 = i;
	}
	return i;
}

uint64_t RedirGraph::randKey()
{
	uint64_t key = 0;
	int shift = std::log2(RAND_MAX);
	for (int i = 0; i < 64; i += shift) {
		key = key << shift;
		key += rand();
	}
	return key;
}
