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
};

void reorder_stages(std::map<int, PipelineComponentBase *> &stages, int nInstances) {
    for (std::pair<const int, PipelineComponentBase *> s : stages) {
        if (!s.second->tasks.empty()) {
            TaskReorder tr(s.second->tasks);
            tr.stage = s.second->getId();
            tr.thinning(nInstances);
        }
    }
}

#endif
