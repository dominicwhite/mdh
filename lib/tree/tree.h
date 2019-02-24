#ifndef MASTERPHYL_TREE_H
#define MASTERPHYL_TREE_H

#include <memory>

class Node {
private:
    int id;
public:
    std::shared_ptr<Node> anc, desc1, desc2;

    explicit Node(int id);
    Node(int id, std::shared_ptr<Node> a, std::shared_ptr<Node> d1, std::shared_ptr<Node> d2);
    ~Node()=default; // Note: deletes recursively (deleting its descendents before itself)

    int get_id(){return id;}

    bool hasDescendents();
};

class Tree {
private:
    int max_id, ntips, nnodes, nbranches;
    std::shared_ptr<Node> root_node;

    std::shared_ptr<Node> getNode(int node_id, std::shared_ptr<Node> current_node);
    int insertNodeAtBranch(int insert_number, int current_branch, std::shared_ptr<Node> anc, std::shared_ptr<Node> desc);

public:

    Tree();
    Tree(std::unique_ptr<std::vector<std::array<int, 2>>>& branch_list, int root_id);
    ~Tree();
    //TODO: copy constructor, move constructor ?
    static std::unique_ptr<Tree> createRandomTree(int ntaxa);

    int getNTips() {return ntips;};
    int getNBranches() {return nbranches;};
    int getNNodes() {return nnodes;};
    int getRootID();
    bool hasNode(int node_id);
    std::unique_ptr<std::vector<std::array<int, 2>>> getBranchList(int start_id = 0, int stop_id = 0);

    bool checkValid(bool verbose = false);

    int addTipRandomly();

    void splitTree(int anc_id, int desc_id, std::unique_ptr<std::array<std::unique_ptr<Tree>, 2>>& subtrees);
////TODO: join two trees: extern Tree *join_trees(Tree *subt1, Tree *subt2, int sister_id, int new_node_id);
////TODO: reroot tree: extern void reroot(Tree *t, int outgroup_id); & extern void reroot_branch(Tree *t, int branch_anc, int branch_des);
//
////TODO: check tree is correct: extern int tree_is_correct(Tree *t);
//
};

#endif
