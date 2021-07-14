#include "RedirGraph.h"

#include <random>
#include <cassert>
#include <iostream>

RedirGraph::RedirGraph(uint32_t size_initial) : 
	size_initial(size_initial)
{
	for (uint32_t i = 0; i < size_initial; i++) {
		RedirNode node;
		nodes.push_back(node);
	}
}

uint32_t RedirGraph::allocNode() {
	nodes.push_back(RedirNode());
	return size() - 1;
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

void RedirGraph::split(uint32_t i, uint32_t& i_split0, uint32_t& i_split1)
{
	assert(i < size() && isRoot(i));

	nodes[i].child0 = allocNode();
	i_split0 = nodes[i].child0;
	nodes[i].child1 = allocNode();
	i_split1 = nodes[i].child1;
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
	// shorten path
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

void RedirGraph::print()
{
	std::cout << "XXXXXXXXXXXXXXXXXXXXXXX" << std::endl;
	for (int i = 0; i < nodes.size(); i++) {
		std::cout << "node " << i << ": ";
		if (nodes[i].child0 != UINT32_MAX) {
			std::cout << nodes[i].child0 << " ";
		}
		if (nodes[i].child1 != UINT32_MAX) {
			std::cout << nodes[i].child1 << " ";
		}
		std::cout << std::endl;
	}
	std::cout << "XXXXXXXXXXXXXXXXXXXXXXX" << std::endl;
}

RedirGraph::RedirNode::RedirNode() :
	child0(UINT32_MAX),
	child1(UINT32_MAX),
	key(randKey())
{
}
