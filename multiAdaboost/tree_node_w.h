#ifndef TreeNode_H
#define TreeNode_H

#include <vector>

struct tree_node{    
	tree_node() : left_constraint(0),right_constraint(0), dim(0), max_split(0), parent(NULL){}       
			double left_constraint;
			double right_constraint;
			int dim;
			int max_split;
			tree_node* parent;
};


using namespace std;

class tree_node_w {
	public:
	int maxsplit;
	private:
	vector<tree_node> nodes;

	public:
		tree_node_w(int max, int iter){
			maxsplit = max;
			nodes.reserve((max*10)*iter); //total ammount of nodes
		}

		tree_node* tree_node_a(){
			tree_node newnode;
			newnode.max_split = maxsplit;
			nodes.push_back(newnode);

			int c = nodes.size();

			return &nodes[c-1];
		}

}; 

#endif 