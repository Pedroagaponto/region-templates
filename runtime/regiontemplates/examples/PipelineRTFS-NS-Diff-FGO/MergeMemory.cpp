#include "MergeMemory.hpp"
#include <algorithm>

using namespace std;

MergeMemory::MergeMemory(PipelineComponentBase* pcb)
{
    for (auto a : pcb->tasks)
    {
        fullmap[a->getId()] = a;
    }
}

bool MergeMemory::sortByCost(int a, int b)
{
    return fullmap[a]->size() > fullmap[b]->size();
}

bool MergeMemory::sortByAccCost(int a, int b)
{
    return fullmap[a]->accumulate > fullmap[b]->accumulate;
}

bool MergeMemory::sortByNumChild(int a, int b)
{
    return fullmap[a]->dependents.size() > fullmap[b]->dependents.size();
}

bool MergeMemory::sortByHeight(int a, int b)
{
    return fullmap[a]->height < fullmap[b]->height;
}

int MergeMemory::reorder()
{
	int resources = 40;
	int root = 1;//pode mover para task
	//set costs
	setCosts(root);
	//passar para array
	vector<int> nodes;
       	allNodes(nodes);
	
	//enquanto custoraiz < requerido
	while (fullmap[root]->accumulate > resources)
	{
		//ordenar por cost
		stable_sort(nodes.begin(), nodes.end(), 
                [this](int a, int b) -> bool{return sortByAccCost(a,b);});
		//ordenar por dependent
		stable_sort(nodes.begin(), nodes.end(), 
				[this](int a, int b) -> bool{return sortByNumChild(a,b);});
		if (fullmap[nodes[0]]->dependents.size() < 2)
			return 1;

		//unir filhos do primeiro
		merge(nodes[0]);

		setCosts(root);
		//atualizar cost de -=retorno
	}
	return 0;
}

int MergeMemory::merge(int parent)
{
	//	filhos para array
    vector<int> children = fullmap[parent]->dependents;

	//	ordenar por cost
	stable_sort(children.begin(), children.end(), 
            [this](int a, int b) -> bool {return sortByAccCost(a,b);});
	//	ordenar por height
	stable_sort(children.begin(), children.end(), 
			[this](int a, int b) -> bool {return sortByHeight(a,b);});

	//	obter folhas do primeiro
	list<int> newParents;
	leaves(children[0], newParents);
	newParents.sort();
	newParents.unique();

	//	mover o segundo para dependente das folhas
	
	for (int elem : fullmap[children[1]]->dependencies)
	{
		fullmap[elem]->accumulate -= fullmap[children[1]]->accumulate;
		fullmap[elem]->dependents.erase(std::remove_if(
					fullmap[elem]->dependents.begin(),
					fullmap[elem]->dependents.end(),
					[children](int a){return a==children[1];}),
				fullmap[elem]->dependents.end());
	}
	for (int elem : newParents)
	{
		fullmap[elem]->dependents.push_back(children[1]);
	}
	
	//updateHeight(parent);

	//	retornar custo do min(childs)


	return 0;	
}

void MergeMemory::updateHeight(int id)
{
	int maxHeight = 0;
	for (int elem : fullmap[id]->dependents)
	{
		updateHeight(elem);
		maxHeight = max(maxHeight, fullmap[elem]->height);
	}

	fullmap[id]->height = maxHeight + 1;
}

int MergeMemory::leaves(int node, list<int> &l)
{
	if(fullmap[node]->dependents.size()==0)
	{
		l.push_back(node);
		return 0;
	}
	else
	{
		for (int elem : fullmap[node]->dependents)
		{
			leaves(elem, l);
		}
	}
	return 0;

}
int MergeMemory::setCosts(int id)
{
	int sum = 0, maxChild = 0, maxHeight = 0;
	if(!fullmap[id]->dependents.empty())
	{
		for (int elem : fullmap[id]->dependents)
		{
			int cost = setCosts(elem);
			sum += cost;
			maxChild = max(maxChild, cost);
			maxHeight = max(maxHeight, fullmap[elem]->height);
		}
	}
	else
	{
		fullmap[id]->isLeaf = true;
	}
	

	int cost = fullmap[id]->cost;
	fullmap[id]->maxChild = maxChild + cost;
	fullmap[id]->accumulate = fullmap[id]->isLeaf ? cost : sum + cost;
	fullmap[id]->height = maxHeight + 1;

	return fullmap[id]->accumulate;
}

int MergeMemory::allNodes(vector<int> &nodes)
{
	for (const auto& node : fullmap) {
		nodes.push_back(node.first);
	}    
	
	return 0;
}
