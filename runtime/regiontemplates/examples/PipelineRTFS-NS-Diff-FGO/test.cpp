#include "test.hpp"
#include <algorithm>
#include <queue>

TaskReorder::TaskReorder(std::list<ReusableTask *> &tasks) : tasks(tasks) {
    for (ReusableTask *rt : tasks) {
        node &n = tree[rt->getId()];
        n.id = rt->getId();
        n.parent = rt->parentTask;
        n.trueParent = rt->parentTask;
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

//void TaskReorder::rebuildList(int id){
//    node &n = tree.at(id);
//    for (auto t : n.children) {
//        rebuildList(t);
//    }
//    tasks.push_back(n.prt);

//}

//void TaskReorder::rebuildList(int id){
//    for (auto node : tree){
//        auto pc = tree.at(node.second.trueParent); //parent children
//        if(std::find(pc.begin(), pc.end(), node.id) == pc.end())
//        {
//            pc.push_back(node.id);
//        }
//    }
//    if(id < 0)
//        return;
//    node &n = tree.at(id);
//    if(n.level = -10)
//        return;

//    if()
//    tasks.push_front(n.prt);
//    n.level = -10;

//    for (auto t : n.children) {
//        rebuildList(t);
//    }

//}

void TaskReorder::rebuildList(int id){

    for (auto& node : tree){
        node.second.children.clear();
    }
    for (auto node : tree){
        if(node.second.trueParent != -1){
            tree.at(node.second.trueParent).children.push_back(node.second.id);
            if(node.second.trueParent != node.second.parent)
                tree.at(node.second.parent).children.push_back(node.second.id);
        }
    }

    // Kahn's algorithm
    std::queue<int> S;
    S.push(id);
    while (!S.empty()) {
        int n = S.front();
        S.pop();
        tasks.push_front(tree.at(n).prt);
        std::cout << n << "\n";
        tree.at(n).level = -10;

        for (auto child : tree.at(n).children){
            if (tree.at(tree.at(child).trueParent).level == -10 &&
                    tree.at(tree.at(child).trueParent).level == -10 &&
                    tree.at(child).level != -10)
                S.push(child);
        }
    }

    if (tasks.size() != tree.size())
    {
        std::cout << "Unable to reorder tasks\n";
        exit(1);
    }
}
//L ← Empty list that will contain the sorted elements
//S ← Set of all nodes with no incoming edge
//while S is non-empty do
//    remove a node n from S
//    add n to tail of L
//    for each node m with an edge e from n to m do
//        remove edge e from the graph
//        if m has no other incoming edges then
//            insert m into S
//if graph has edges then
//    return error   (graph has at least one cycle)
//else
//    return L   (a topologically sorted order)

void TaskReorder::writeTree() {
    for (auto n : tree) {
        n.second.prt->fakeParent = n.second.parent;
    }
    
    tasks.clear();
    rebuildList(root);
    
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

    int parent = tree.at(id).trueParent;
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
        }
        nodes = filterLevel(i++);
    }
}

int TaskReorder::child2promote(int id, int trueParent) {
    int ret = 0;
    for (auto n : tree.at(id).children){
        if ((ret = child2promote(n, trueParent)) != 0)
            return ret;
    }

    if (tree.at(id).trueParent == trueParent)
        return id;

    return 0;
}

void TaskReorder::thickening(int minWidth) {
    int i = 5;
    auto nodes = filterLevel(i++);

    while (!nodes.empty()) {
        for (int j = 0; j < nodes.size(); j++){
            if(nodes.size() < minWidth){
                int child = child2promote(nodes[j], tree.at(nodes[j]).trueParent);
                if (child != nodes[j]){
                    changeParent(child, tree.at(nodes[j]).parent);
                    nodes.push_back(child);
                    j--;
                }
            }
        }
        nodes = filterLevel(i++);
    }
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
