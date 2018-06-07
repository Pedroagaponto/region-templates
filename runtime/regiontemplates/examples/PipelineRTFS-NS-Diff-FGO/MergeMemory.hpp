#pragma once
#include <iostream>  

#include <vector>
#include <list>
#include <map>


#include "ReusableTask.hpp"
#include "PipelineComponentBase.h"
#include "RTPipelineComponentBase.h"



class MergeMemory {
	public:

        MergeMemory(ReusableTask& task);
		int reorder();
		int setCosts(int id);
		int allNodes(std::vector<int> &nodes);

        std::map<int, ReusableTask*> fullmap;
	private:
		int merge(int parent);
		int leaves(int node, std::list<int> &l);
        void updateHeight(int id);
        bool sortByCost(int a, int b);
        bool sortByAccCost(int a, int b);
		bool sortByNumChild(int a, int b);
		bool sortByHeight(int a, int b);
};

