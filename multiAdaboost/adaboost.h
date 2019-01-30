#ifndef ADABOOST_H
#define ADABOOST_H

#include <algorithm>
#include <numeric>
#include <vector>
#include <iterator>
#include <math.h>
#include <tuple>
#include <tbb\parallel_for.h>

#include "tree_node_w.h"

using namespace std;

	//other

class Boosting {

	private:
	int MaxIter;

public:
	Boosting(int max_iter);

	tuple<vector<tree_node*>,vector<float>,vector<float>> RealAdaBoost(tree_node_w* WeakLrn, vector<vector<float>> Data, vector<int> Labels, int width, int height);
	vector<float> Classify(vector<tree_node*> Learners, vector<float> Weights, vector <vector <float> > ControlData, int width, int height);
	vector<int> sign(vector<float> input, int width, int height);
	vector<int> sign4(vector<float> input1, vector<float> input2, vector<float> input3, vector<float> input4,  vector<float> input5, vector<float> input6, int width, int height, int* sign_nb, std::vector<int> bg);

private:
	vector<tree_node*> train(tree_node_w* WeakLrn, vector<vector<float>> Data, vector<int> Labels, vector<float> Weights, int width, int height);
	tuple<tree_node*,tree_node*,float> do_learn_nu(tree_node_w* WeakLrn, vector<vector<float>> Data, vector<int> Labels, vector<float> Weights, int width, int height, tree_node* Parent = NULL);
	vector<int> calc_output(tree_node* WeakLrn, vector<vector<float>> Data, int width, int height);
	tuple<vector<float>,vector<float>> genWeights(tree_node* left, tree_node* right, vector<vector<float>> Data, vector<int> Labels, vector<float> Weights, int width, int height);
	vector<int> cmpData(vector<float> XData, int condition, float value);
	vector<int> findLabels(vector<int> Labels, int condition, int height);
	vector<int> findLabels(vector<int> Labels, vector<int> condition, int height);
	

private:

	template<class T> struct index_cmp {
		index_cmp(const T arr) : arr(arr) {}
		bool operator()(const size_t a, const size_t b) const
		{ return arr[a] < arr[b]; }
		const T arr;
	};

};

#endif 