#ifndef MASTERPHYL_TREE_H
#define MASTERPHYL_TREE_H

#include <memory>
#include <vector>

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

// TODO: template class that accepts different types of nodes -> branch lengths, and variable descendent numbers
class Tree {
private:
    int max_id, ntips, nnodes, nbranches;
    std::shared_ptr<Node> root_node;

    std::shared_ptr<Node> getNode(int node_id, std::shared_ptr<Node> current_node);
    int insertNodeAtBranch(int insert_number, int current_branch, std::shared_ptr<Node> anc, std::shared_ptr<Node> desc);
    void copySubtree(Tree& subtree, std::shared_ptr<Node>& subtree_node, Node& original_node, int start_node, int stop_node);
    void copyJoinedSubtrees(std::shared_ptr<Node>& new_tree_node, Node& copied_node, Tree& branch_tree, int sister_id, int new_node_id = 0);
    int recursive_reroot(std::shared_ptr<Node>& current_node, std::shared_ptr<Node>& new_outgroup);
public:

    Tree();
    Tree(std::unique_ptr<std::vector<std::array<int, 2>>>& branch_list, int root_id);
    Tree(Tree& base_tree, Tree& branch_tree, int sister_id, int new_node_id);
    ~Tree();
    //TODO: copy constructor, move constructor ?
    static std::unique_ptr<Tree> createRandomTree(int ntaxa);

    int getNTips() {return ntips;};
    int getNBranches() {return nbranches;};
    int getNNodes() {return nnodes;};
    int getRootID();
    bool hasNode(int node_id);
    // std::vector<int> getTips();
    // std::vector<int> getNodes();
    std::unique_ptr<std::vector<std::array<int, 2>>> getBranchList(int start_id = 0, int stop_id = 0);

    bool checkValid(bool verbose = false);

//    void addTipNextTo(int id, int sibling_id);
    void addTipFrom(int id, int anc_id); // TODO: test this method better
    int addTipRandomly();
    // TODO: add tips in other ways.

    void splitTree(int anc_id, int desc_id, std::array<std::unique_ptr<Tree>, 2>& subtrees);
    // Could also add faster detach/reattach subtree methods, which don't create copies of tree -> faster tree search?
    // TODO: reattach subtrees?
    void reroot(int outgroup_node);

    std::shared_ptr<Node> getRootNode(); //TODO: add tests
};

#endif
