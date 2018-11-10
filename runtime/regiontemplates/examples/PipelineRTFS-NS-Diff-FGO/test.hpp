#ifndef TEST_HPP
#define TEST_HPP
#include <list>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include "ReusableTask.hpp"

class TaskReorder {
  public:
    TaskReorder(std::list<ReusableTask *> &tasks);
    ~TaskReorder();

    void print(int origin = 0, int level = 0) const;

    void thinning(int maxWidth);

    int stage = 0;
  private:
    class node {
      public:
        int id = 0;
        int parent = 0;
        std::vector<int> children;
        int height = 0;
        int level = 0;
        ReusableTask *prt = nullptr;
    };

    std::unordered_map<int, node> tree;
    int root = 0;
    std::list<ReusableTask *> &tasks;

    void changeParent(int child, int newParent);
    int updateHeight(int id);
    void updateLevel(int id, int level);
    void removeChild(int child, std::vector<int> &children);
    int shortestLeaf(std::vector<int> nodes);
    std::vector<int> brothers(int id);
    std::vector<int> filterLevel(int level);
    int child2remove(std::vector<int> nodes);
    void printDOT(std::string filename);
    void writeTree();
    void rebuildList(int id);
};

void reorder_stages(const std::map<int, PipelineComponentBase *> stages, const int nInstances) {
    for (auto s : stages) {
        if (!s.second->tasks.empty()) {
            TaskReorder tr(s.second->tasks);
            tr.stage = s.second->getId();
            tr.thinning(nInstances);
        }
    }
}

void reorder_stages_parallel(const std::map<int, PipelineComponentBase *> stages, const int nInstances) {
    #pragma omp parallel for
    for(int i = 0; i < stages.size(); i++) {
        auto it = stages.begin();
        advance(it, i);
        if (!it->second->tasks.empty()) {
            TaskReorder tr(it->second->tasks);
            tr.stage = it->second->getId();
            tr.thinning(nInstances);
        }
    }
}
#endif
