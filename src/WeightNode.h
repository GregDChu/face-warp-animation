#pragma once

#include <vector>

using namespace std;

#define EPSILON 0.001

class WeightNode
{

public:
	int id;
	float weight[10] = { };
	bool visited;
	vector<WeightNode*> adjacent;
	WeightNode(int id)
	{
		this->id = id;
		visited = false;
	}
	int getId() { return id; }
	vector<WeightNode*> getAdj() { return adjacent; }
	float getWeight(int anchor) { return weight[anchor]; }
	void setWeight(int anchor, float weight)
	{
		this->weight[anchor] = weight;
	}
	bool wasVisited() { return visited; }
	void markVisited() { visited = true; }
	void unmarkVisited() { visited = false; }
	void addAdjacent(WeightNode *node)
	{
		for (int i = 0; i < adjacent.size(); i++)
		{
			if (adjacent.at(i)->getId() == node->getId())
			{
				return;
			}
		}
		adjacent.push_back(node);
	}
};