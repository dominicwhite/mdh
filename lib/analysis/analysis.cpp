#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>

#include "matrix.h"
#include "tree-orig.h"
#include "dynamic_homology.h"
#include "rf.h"
#include "output.h"

#define DEBUG 1 //0 means no debugging output, 1 means basic debugging, 2 means detailed debugging

float continuous_downpass(Tree *t, Partition *p, int char_idx, Node *n, float *node_states){
    if (n->desc1 != NULL && n->desc2 != NULL){
        float d1 = continuous_downpass(t, p, char_idx, n->desc1, node_states);
        float d2 = continuous_downpass(t, p, char_idx, n->desc2, node_states);
        //printf(" On inner node %i, calculating...\n", n->id);
        float d1_u, d1_l, d2_u, d2_l, c=0;
        d1_l = node_states[n->desc1->id*2];
        d1_u = node_states[n->desc1->id*2+1];
        d2_l = node_states[n->desc2->id*2];
        d2_u = node_states[n->desc2->id*2+1];
        if (d1_l <-0.01 || d2_l < -0.01){ // if one or more characters missing
            //printf("### a char is missing in %i\n", p->data_type);
		    if (d1_l <-0.01 && d2_l > -0.01){
		        node_states[n->id*2] = d2_l;
		        node_states[n->id*2+1] = d2_u;
		    } else{
		        if (d1_l > -0.01 && d2_l < -0.01){
		            node_states[n->id*2] = d1_l;
		            node_states[n->id*2+1] = d1_u;
		        } else{
		            node_states[n->id*2] = -1;
		            node_states[n->id*2+1] = -1;
		        }
		    }
        } else{ // if neither is missing
            //printf("### no missing\n");
		    if (d1_l > d2_u){ // TODO check literature for case when node's state boundaries are equal ie 1-2 & 2-3
		        c = d1_l - d2_u;
		        node_states[n->id*2] = d2_u;
		        node_states[n->id*2+1] = d1_l;
		    } else{
		        if (d1_u < d2_l){
		            c = d2_l - d1_u;
		            node_states[n->id*2] = d1_u;
		            node_states[n->id*2+1] = d2_l;
		        } else{
		            if (d1_l > d2_l) node_states[n->id*2] = d1_l;
		            else node_states[n->id*2] = d2_l;
		            if (d1_u > d2_u) node_states[n->id*2+1] = d2_u;
		            else node_states[n->id*2] = d1_u;
		        }
		    }
	    }
        return d1+d2+c;
    } else{
        //printf(" On tip %i, entering values into arrays...\n", n->id);
        node_states[n->id*2] = p->states[n->id][char_idx*2];
        node_states[n->id*2+1] = p->states[n->id][char_idx*2+1];
        return 0;
    }
    
}


float continuous_optimization(Tree *t, Partition *p){
    float cumulative_cost = 0.0;
    for (int char_idx = 0; char_idx < p->nchar; char_idx++){
        //printf("Calculating char %i\n", char_idx);
        float node_state_ranges[2*(t->ntaxa*2 - 1)];
        cumulative_cost += continuous_downpass(t, p, char_idx, t->root_node, &node_state_ranges[0]);
    }
    return cumulative_cost;
}



int discrete_optimization(Tree *t, Partition *p){
    return continuous_optimization(t, p);
}


float get_cost(Tree *t, Matrix *m){
    //print_matrix(m);
    float total_cost = 0.0;
    for (int idx = 0; idx < m->npart; idx++){
        //printf("Working on partition %i of type %i\n", idx+1, m->partitions[idx]->data_type);
        if (m->partitions[idx]->data_type == 0){
            total_cost = total_cost+discrete_optimization(t, m->partitions[idx]);
        } else {
            if (m->partitions[idx]->data_type == 1){
                total_cost = total_cost+continuous_optimization(t, m->partitions[idx]);
            } else{
                if (m->partitions[idx]->data_type == 2){
                    total_cost = total_cost+dynamic_homology(t, m->partitions[idx], 1.0);
                }
                else{
                    assert(!"Unknown partition type");
                }
            }
        }
    }
    return total_cost;
}


TreeSet spr(Tree *start_tree, Matrix *m, float current_best_cost, int max_trees){
    TreeSet spr_trees;
    spr_trees.ntrees = 0;
    //printf("Checking start tree\n");
    tree_is_correct(start_tree);
    int *all_branches = get_branch_list(start_tree);
    for (int idx=0; idx <  2*start_tree->ntaxa-2 ; idx++){
        int branch_anc = all_branches[2*idx];
        int branch_des = all_branches[2*idx+1];
        
        //printf("\n\n\n%i - Starting SPR at branch %i to %i\n\n Starting tree:\n", idx, branch_anc, branch_des);
        //print_tree(start_tree, start_tree->root_node, 0);
        
        //printf("\n\n Splitting trees at branch %i -> %i\n", branch_anc, branch_des);
        Tree *subtrees[2] = {NULL, NULL};
        split_tree(start_tree, branch_anc, branch_des, subtrees);
        //printf("\n Created subtrees, with %i and %i taxa\n", subtrees[0]->ntaxa, subtrees[1]->ntaxa);
        //printf("Subtree1:\n");
        //print_tree(subtrees[0], subtrees[0]->root_node, 0);
        //print_nodes(subtrees[0], subtrees[0]->root_node);
        //printf("\nSubtree2:\n");
        //print_tree(subtrees[1], subtrees[1]->root_node, 0);
        //print_nodes(subtrees[1], subtrees[1]->root_node);
        //printf("\n\n");
        
        //printf("\n  Print of Start tree after making subtrees from it:\n");
        //print_tree(start_tree, start_tree->root_node, 0);
        //print_nodes(start_tree, start_tree->root_node);
        //printf("\n\n");
        
        if (subtrees[0]->ntaxa > 1){  // if subtree 1 has more than 1 tip:
            int *branches_to_attach_to = get_branch_list(subtrees[0]);
            
            for (int idx=0; idx < 2*subtrees[0]->ntaxa-2 ; idx++){  // for branch in subtree branches
                //printf("\n\nREJOINING\nReattaching as sister to node %i ", branches_to_attach_to[idx*2+1]);
                //printf("on branch %i to %i\n", branches_to_attach_to[idx*2], branches_to_attach_to[idx*2+1]);
                //printf("\n  Print of Start tree after copying subtrees but before joing them:\n");
                //print_tree(start_tree, start_tree->root_node, 0);
                
                // reattach subt2 at this branch
                //printf("\n\nStarting join_trees function:\n");
                Tree *rearranged_tree = join_trees(subtrees[0], subtrees[1], branches_to_attach_to[2*idx+1], branch_anc);
                //printf("\n  Print of joined tree:\n");
                //print_tree(rearranged_tree, rearranged_tree->root_node, 0);
                //printf("\n  Print of Start tree:\n");
                //print_tree(start_tree, start_tree->root_node, 0);
                //printf("Joined subtrees\n");
                
                // make sure it is rooted in the right place
                reroot(rearranged_tree, 0); //TODO make root an argument
                //printf("Finished rerooting tree\n");
                //printf("\n  Print of Start tree:\n");
                //print_tree(start_tree, start_tree->root_node, 0);
                
                // get cost
                float tree_cost = get_cost(rearranged_tree, m);
                //printf("Calculated cost: %f\n", tree_cost);
                
                // based on cost, figure out what to do
                if (tree_cost > current_best_cost){
                    free_tree(rearranged_tree);
                }
                else{
                    if (tree_cost == current_best_cost){
                        //printf("Tree has same length\n");
                        int is_same = 0;
                        for (int i = 0; i < spr_trees.ntrees; i++){ // check tree is unique
                            is_same = trees_different(spr_trees.trees[i], rearranged_tree);
                            printf(" %i ", is_same);
                            if (is_same == 1) break;
                        }
                        if (is_same == 0){
                            //printf("  Tree is new\n");
                            spr_trees.trees[spr_trees.ntrees] = rearranged_tree;
                            spr_trees.ntrees++;
                            spr_trees.tcost = tree_cost;
                            if (spr_trees.ntrees >= max_trees){
                                free_tree(subtrees[0]);
                                free_tree(subtrees[1]);
                                free(all_branches);
                                return spr_trees;
                            }
                        } else{
                            //printf("  Tree is same\n");
                            free_tree(rearranged_tree);
                        }
                    } else{
                        for(int idx=0; idx < spr_trees.ntrees; idx++){
                            free_tree(spr_trees.trees[idx]);
                            spr_trees.trees[idx] = NULL;
                        }
                        current_best_cost = tree_cost;
                        spr_trees.trees[0] = rearranged_tree;
                        spr_trees.ntrees = 1;
                        spr_trees.tcost = tree_cost;
                    }
                }
            }
            
            free(branches_to_attach_to);
            
        }
        free_tree(subtrees[0]);
        free_tree(subtrees[1]);
        
        // free any created and unneeded structures
    }
    
    
    //printf("\n");
    
    free(all_branches);
    return spr_trees;
}


TreeSet run_analysis(Matrix *m, int spr_rounds, int max_trees){ // TODO: take root and max trees
    Tree *rand_tree = make_random_tree(m->ntaxa);
    float analysis_best_cost = 2 * get_cost(rand_tree, m);
    //printf("Made random tree\n");
    assert(tree_is_correct(rand_tree));
    TreeSet analysis_best_trees;
    analysis_best_trees.ntrees = 0;
    for (int iter = 0; iter < spr_rounds; iter++){
        //printf("On SPR round %i\n", iter);
        TreeSet iter_trees = spr(rand_tree, m, analysis_best_cost, 100);
        if (iter_trees.ntrees > 0){
            //printf("Cost of %i round %i trees is %f\n", iter_trees.ntrees, iter+1, iter_trees.tcost);
            if (iter_trees.tcost < analysis_best_cost || iter == 0){
                //printf("Trees are better\n\n");
                for (int idx = 0; idx < analysis_best_trees.ntrees; idx++){
                    free_tree(analysis_best_trees.trees[idx]);
                }
                analysis_best_cost = iter_trees.tcost;
                analysis_best_trees = iter_trees;
            } else{
                if ( analysis_best_cost == iter_trees.tcost){ // if same as existing
                    //printf("Trees are equal to previous\n");
                    for (int i = 0; i < iter_trees.ntrees; i++){ // check tree is unique
                        int is_same = 0;
                        for (int j = 0; j < analysis_best_trees.ntrees; j++){
                            is_same = trees_different(iter_trees.trees[i], analysis_best_trees.trees[j]);
                            if (is_same == 1) break;
                        }
                        if (is_same == 0){
                            //printf("Tree is new\n");
                            analysis_best_trees.trees[analysis_best_trees.ntrees] = iter_trees.trees[i];
                            analysis_best_trees.ntrees++;
                            if (analysis_best_trees.ntrees >= max_trees) {
                                for (int remaining = i+1; remaining < iter_trees.ntrees; remaining++){
                                    free_tree(iter_trees.trees[remaining]);
                                }
                                break;
                            }
                        } else{
                            //printf("Tree already exists\n");
                            free_tree(iter_trees.trees[i]);
                        }
                    }
                } else{ // if worse
                    //printf("Trees are worse\n\n");
                    for (int idx = 0; idx < iter_trees.ntrees; idx++){
                        free_tree(iter_trees.trees[idx]);
                    }
                }
                
                
                
            // TODO: join treesets if same cost (up to point where max trees reached)
            //       and check that trees are not already in the treeset
            
            }
        }
        if (analysis_best_trees.ntrees >= max_trees) break;
    }
    free_tree(rand_tree);
    //printf("Free'd tree\n");
    return analysis_best_trees;
}


void run_analysis_shared(float states[], int ntaxa, int nchar, int npart, int part_types[], int chars_per_part[], int segs_per_taxon_per_part[], int chars_per_seg[], int spr_rounds, int max_trees){
    srand(time(NULL));
    Matrix *new_m = make_matrix(ntaxa, nchar, npart, part_types, chars_per_part, segs_per_taxon_per_part, chars_per_seg, states);
    TreeSet final_trees = run_analysis(new_m, spr_rounds, max_trees); 
    printf("\n\nCost of %i tree(s) is %f\n\n", final_trees.ntrees, final_trees.tcost);
    FILE *f = fopen("output.tre", "w");
    assert(f != NULL);
    for (int idx = 0; idx < final_trees.ntrees; idx++){
        char *tre = to_newick(final_trees.trees[idx], final_trees.trees[idx]->root_node);
        printf("%s\n",  tre);
        print_tree(final_trees.trees[idx], final_trees.trees[idx]->root_node, 0);
        fprintf(f, "%s\n", tre);
        free_tree(final_trees.trees[idx]);
    }
    fclose(f);
    Matrix_destroy(new_m);
}


























