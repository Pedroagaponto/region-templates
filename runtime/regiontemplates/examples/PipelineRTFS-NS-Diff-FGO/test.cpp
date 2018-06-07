#include "test.hpp"
#include <algorithm>

TaskReorder::TaskReorder(std::list<ReusableTask *> &tasks) {
    for (ReusableTask *rt : tasks) {
        node &n = tree[rt->getId()];
        n.id = rt->getId();
        n.parent = rt->parentTask;
        n.prt = rt;

        if (n.parent != -1)
            tree[rt->parentTask].children.push_back(n.id);
        else
            root = rt->getId();
    }

    updateLevel(root, 0);
    updateHeight(root);
}
TaskReorder::~TaskReorder() { writeTree(); }

void TaskReorder::writeTree() {
    for (auto n : tree) {
        n.second.prt->parentTasks.clear();
        n.second.prt->parentTasks.push_back(n.second.parent);
        n.second.prt->parentTasks.push_back(n.second.prt->parentTask);
    }
}

/*void TaskReorder::print(int origin, int level) const {
    if (origin < 0) return;
    if (!origin) origin = root;

    for (int i = 0; i < level; ++i) std::cout << '\t';

    std::cout << origin << std::endl;

    for (int rt : tree.at(origin)) print(rt, level + 1);
}*/

void TaskReorder::printDOT(std::string filename) {
    static int i = 0;
    std::ofstream os(filename + "_" + std::to_string(i) + ".dot");

    os << "digraph graphname {" << std::endl;
    os << "subgraph cluster" << i << " {" << std::endl;
    for (auto t : tree) {
        os << t.first << " [shape=box,label=\"" << t.first << "\\n"
           << t.second.prt->getTaskName() << "\\n"
           << t.second.level << ":" << t.second.height;
        os << "\\nsize: " << t.second.prt->size() << "\"]; " << std::endl;
        if (t.second.prt->parentTask != -1)
            os << t.second.prt->parentTask << " -> " << t.first << ";\n";

        if (t.second.prt->parentTask != t.second.parent)
            os << t.second.parent << " -> " << t.first << " [color=blue];\n";
    }
    os << "}" << std::endl;
    os << "}" << std::endl;
}

int TaskReorder::child2remove(std::vector<int> nodes) {
    int remove = 0;
    int lenght = 1e9;
    for (auto n : nodes) {
        int parent = tree.at(n).parent;
        if ((tree.at(parent).children.size() > 1) &&
            (tree.at(n).height < lenght)) {
            lenght = tree.at(n).height;
            remove = n;
        }
    }

    return remove;
}

std::vector<int> TaskReorder::filterLevel(int level) {
    std::vector<int> ret;

    for (auto t : tree)
        if (t.second.level == level) ret.push_back(t.first);

    return ret;
}

std::vector<int> TaskReorder::brothers(int id) {
    std::vector<int> ret;

    int parent = tree.at(id).parent;
    for (auto child : tree.at(parent).children)
        if (child != id) ret.push_back(child);

    return ret;
}

int TaskReorder::shortestLeaf(std::vector<int> nodes) {
    int shortest = 0;
    while (!nodes.empty()) {
        int minHeight = 1e9;
        for (auto n : nodes) {
            if (tree.at(n).height < minHeight) {
                minHeight = tree.at(n).height;
                shortest = n;
            }
        }
        nodes = tree.at(shortest).children;
    }

    return shortest;
}

void TaskReorder::removeChild(int child, std::vector<int> &children) {
    children.erase(std::remove_if(children.begin(), children.end(),
                                  [child](int x) { return x == child; }),
                   children.end());
}

void TaskReorder::updateLevel(int id, int level) {
    tree.at(id).level = level;
    for (auto n : tree.at(id).children) {
        updateLevel(n, level + 1);
    }
}

int TaskReorder::updateHeight(int id) {
    int maxHeight = 0;
    for (auto n : tree.at(id).children) {
        maxHeight = std::max(updateHeight(n), maxHeight);
    }
    maxHeight++;
    tree.at(id).height = maxHeight;
    return maxHeight;
}

void TaskReorder::changeParent(int child, int newParent) {
    int oldParent = tree.at(child).parent;
    removeChild(child, tree.at(oldParent).children);

    tree.at(newParent).children.push_back(child);
    tree.at(child).parent = newParent;

    updateLevel(root, 0);
    updateHeight(root);
}

void TaskReorder::thinning(int maxWidth) {
    int i = 0;
    auto nodes = filterLevel(i++);
    while (!nodes.empty()) {
        int j = 0;
        while (nodes.size() > maxWidth) {
            int child = child2remove(nodes);
            int newParent = shortestLeaf(brothers(child));

            changeParent(child, newParent);
            removeChild(child, nodes);

            //            if (stage == 1293)
            // printDOT("dot/s1285_" + std::to_string(i + 10) +
            //       std::to_string((j++) + 10));
        }
        nodes = filterLevel(i++);
    }

    //printDOT("abcd_");
}

// void reorder_stage(std::list<ReusableTask *> &tasks) {
//    for (ReusableTask *t : tasks) {
//        if (t->getTaskName().compare("TaskSegmentation6") == 0) {
//            segment[t->parentTask->parentTask->parentTask->parentTask
//                        ->getId()] = t->getId();
//        }

//        int i;
//        for (auto s : segment) {
//            i++;
//        }
//    }
//}
