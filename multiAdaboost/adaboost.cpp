#include "adaboost.h"
#include <iostream>

	int op_negative (int i) { return -i; }
	float op_exp (float i) { return exp((float)i); }

Boosting::Boosting(int max_iter){
	MaxIter = max_iter;
}


tuple<vector<tree_node*>,vector<float>,vector<float>> Boosting::RealAdaBoost(tree_node_w* WeakLrn, vector<vector<float>> Data, vector<int> Labels, int width, int height){

	tuple<vector<tree_node*>,vector<float>,vector<float>> retVal;

	vector<tree_node*> Learners;
	vector<float> Weights;
	vector<float> distr(height);
	

	distr.resize(height);
    fill (distr.begin(),distr.end(),(float)1/height); 
	vector<float> final_hyp(height);

	for(int It = 0; It < MaxIter; It++){

		vector<tree_node*> nodes = train(WeakLrn, Data, Labels, distr, width, height);

		for(int i =0; i < (int)nodes.size(); i++){

			tree_node* curr_tr = nodes[i];

			vector<int> step_out = calc_output(curr_tr, Data, width, height);

			//count positive samples
			std::vector<int> postive = findLabels(Labels, 1, height); 
			//count negative samples
			std::vector<int> negative = findLabels(Labels, -1, height); 

			std::vector<int> temp(height);
			std::vector<float> s(height);
			std::vector<float> sRev(height);

			// S
			std::transform (postive.begin(), postive.end(), step_out.begin(), temp.begin(), std::multiplies<int>());
			std::transform(temp.begin(),temp.end(),distr.begin(),s.begin(),std::multiplies<float>());
			float s1 = std::accumulate(s.begin(),s.end(),(float)0,std::plus<float>());
			
			std::transform (negative.begin(), negative.end(), step_out.begin(), temp.begin(), std::multiplies<int>());
			std::transform(temp.begin(),temp.end(),distr.begin(),s.begin(),std::multiplies<float>());
			float s2 = std::accumulate(s.begin(),s.end(),(float)0,std::plus<float>());
			
			if (s1 <= (float)0 && s2 <= (float)0)
				continue;

			float Alpha = float(0.5*log((s1 + FLT_EPSILON) / (s2+ FLT_EPSILON)));

			Weights.push_back(Alpha);

			Learners.push_back(curr_tr);
			vector<float> temp_hyp(height);

			std::transform (temp_hyp.begin(), temp_hyp.end(), step_out.begin(), temp_hyp.begin(), std::plus<float>());
			std::transform(temp_hyp.begin(), temp_hyp.end(), temp_hyp.begin(), bind2nd(std::multiplies<float>(), Alpha));  
			std::transform (final_hyp.begin(), final_hyp.end(), temp_hyp.begin(), final_hyp.begin(), std::plus<float>());
		}

		std::transform(Labels.begin(), Labels.end(), final_hyp.begin(), distr.begin(), std::multiplies<float>());
		std::transform(distr.begin(), distr.end(), distr.begin(), bind2nd(std::multiplies<float>(), (float)-1));  
		std::transform (distr.begin(), distr.end(), distr.begin(), op_exp);

		float Z = std::accumulate(distr.begin(),distr.end(),(float)0, std::plus<float>());

		//were not getting any better
		if(Z == 0)
			break;

		std::transform(distr.begin(), distr.end(), distr.begin(), bind2nd(std::divides<float>(), Z)); 
	}

	get<0>(retVal) = Learners;
	get<1>(retVal) = Weights;
	get<2>(retVal) = final_hyp;

	return retVal;
}

vector<tree_node*> Boosting::train(tree_node_w* WeakLrn, vector<vector<float>> Data, vector<int> Labels, vector<float> Weights, int width, int height){

	//init
	//int max_split = WeakLrn.max_split;
	int max_split = WeakLrn->maxsplit;
	float spit_error;

	std::vector<tree_node*> WeakLrns; //vector of treenodes retVal
	WeakLrns.reserve(max_split+1);

	//second init
	std::vector<tree_node*> splits;
	splits.reserve(max_split+1);
	std::vector<float> split_errors;
	split_errors.reserve(max_split+1);
	std::vector<float> deltas;
	deltas.reserve(max_split+1);
	std::vector<float> errors;
	errors.reserve(max_split+1);

	tuple<tree_node*,tree_node*,float> output = do_learn_nu(WeakLrn, Data, Labels, Weights, width, height, NULL);
	spit_error = get<2>(output);

	WeakLrns.push_back(get<0>(output));
	WeakLrns.push_back(get<1>(output));

	//gen weights
	tuple<std::vector<float>,std::vector<float>> outWeights =  genWeights(get<0>(output), get<1>(output), Data, Labels, Weights, width, height);

	
	errors.push_back(*std::min_element(get<0>(outWeights).begin(),get<0>(outWeights).end()));
	errors.push_back(*std::min_element(get<1>(outWeights).begin(),get<1>(outWeights).end()));
	

	if(get<1>(outWeights)[0] == 0 && get<1>(outWeights)[1] == 0)
		return WeakLrns;

	if(get<0>(outWeights)[0] == 0 && get<0>(outWeights)[1] == 0)
		return WeakLrns;

		std::vector<int> IDX;

		for (int i = 0; i < (int)errors.size(); ++i)
			IDX.push_back(i);

		std::sort(IDX.begin(), IDX.end(), index_cmp<std::vector<float>&>(errors));

	std::sort(errors.begin(), errors.end());
	reverse(errors.begin(), errors.end());
	std::reverse(IDX.begin(), IDX.end());
	
	//move the largest node to the left //temp
	std::vector<tree_node*> WeakLrnsTemp = WeakLrns;

	WeakLrns[0] = WeakLrnsTemp[IDX[0]];
	WeakLrns[1] = WeakLrnsTemp[IDX[1]];

	
	for(int i = 1; i < max_split; i++){
		for(int j = 0; j < (int)errors.size(); j++){

			if((int)deltas.size() > j)
				continue;
		
			tree_node* max_WeakLrn = WeakLrns[j];
			std::vector<int> max_WeakLrn_out = calc_output(max_WeakLrn,Data, width, height); //check this

			std::vector<int> mask;
			//pushback(postiverowID);
			for(int i = 0; i < (int)max_WeakLrn_out.size(); i++){
				if(max_WeakLrn_out[i] == 1)
					mask.push_back(i);
			}

			//remove colum ids from Data that are not in mask
			vector<vector<float>> DataPass(width);
			vector<float> DataLine, DataPassLine(mask.size());

			for(int i = 0; i < (int)Data.size(); i++){
				DataLine = Data[i];
				#pragma parallel
				#pragma loop count min(1024)
				for(int m = 0; m < (int)mask.size(); m++){
					 DataPassLine[m] = DataLine[mask[m]];
				}
				DataPass[i] = DataPassLine;
			}

			vector<int> LabelsPass(mask.size());
			vector<float> WeightsPass(mask.size());
			#pragma parallel
			#pragma loop count min(1024)
			for(int m = 0; m < (int)mask.size(); m++){
					 LabelsPass[m] = Labels[mask[m]];
					 WeightsPass[m] = Weights[mask[m]];
				}

			tuple<tree_node*,tree_node*,float> output = do_learn_nu(WeakLrn, DataPass, LabelsPass, WeightsPass, width, mask.size(), max_WeakLrn);
			
			spit_error = get<2>(output);

			tuple<std::vector<float>,std::vector<float>> outWeights =  genWeights(get<0>(output), get<1>(output), Data, Labels, Weights, width, height);

		splits.push_back(get<0>(output));
		splits.push_back(get<1>(output));

		if( (get<1>(outWeights)[0] + get<1>(outWeights)[1]) == (float)0 || ( get<0>(outWeights)[0] +  get<0>(outWeights)[1] ) == (float)0)
			deltas.push_back(0);
		else
			deltas.push_back(errors[j] - spit_error);

		split_errors.push_back(*std::min_element(get<0>(outWeights).begin(),get<0>(outWeights).end()));
		split_errors.push_back(*std::min_element(get<1>(outWeights).begin(),get<1>(outWeights).end()));

		}

		if(*std::max_element(deltas.begin(),deltas.end()) == 0)
			return WeakLrns;

		std::vector<float>::iterator it;

			reverse(deltas.begin(), deltas.end());
			// iterator to vector element:
			it = find (deltas.begin(), deltas.end(), (float)*std::max_element(deltas.begin(),deltas.end()));
			int best_split = abs(it-deltas.end()+1);
			reverse(deltas.begin(), deltas.end());
	
		std::vector<int> cut_vec;

		for (int i=0; i<best_split; i++)
			cut_vec.push_back(i);
		for (int i=best_split+1; i<(int)errors.size(); i++)
			cut_vec.push_back(i);

		WeakLrnsTemp = WeakLrns;

		//keep only the nodes, errors and delras in cut_vec
		for(int i =0; i < (int)cut_vec.size(); i++)
		{
		WeakLrns[i] = WeakLrnsTemp[cut_vec[i]];
		errors[i] = errors[cut_vec[i]];
		deltas[i] = deltas[cut_vec[i]];
		}
		while(WeakLrns.size() > (int)cut_vec.size()){
			WeakLrns.pop_back();
			errors.pop_back();
			deltas.pop_back();
			}

		//clear cut_vec
		cut_vec.clear();

		//add two nodes
		WeakLrns.push_back(splits[2*best_split]);
		WeakLrns.push_back(splits[2*best_split+1]);

		//add two erros
		errors.push_back(split_errors[2*best_split]);
		errors.push_back(split_errors[2*best_split+1]);

		for (int i=0; i<((best_split))*2; i++)
			cut_vec.push_back(i);
		for (int i=((best_split)+1)*2; i<(int)split_errors.size(); i++)
			cut_vec.push_back(i);
			  
		//keep only the split_errors and splits in cut_vec
		for(int i =0; i < (int)cut_vec.size(); i++)
		{
		split_errors[i] = split_errors[cut_vec[i]];
		splits[i] = splits[cut_vec[i]];
		}
		split_errors.resize((int)cut_vec.size());
		splits.resize((int)cut_vec.size());
		split_errors.shrink_to_fit();
		splits.shrink_to_fit();

		//clear cut_vec
		cut_vec.clear();

	}

	return WeakLrns;
}

tuple<tree_node*,tree_node*,float> Boosting::do_learn_nu(tree_node_w* WeakLrn, vector<vector<float>> Data, vector<int> Labels, vector<float> Weights, int width, int height, tree_node* Parent){

	tuple<tree_node*,tree_node*,float> retVal;

	tree_node *tree_node_left = WeakLrn->tree_node_a();
	tree_node *tree_node_right = WeakLrn->tree_node_a();

	if(Parent != NULL)
	{
		tree_node_left->parent = Parent;
		tree_node_right->parent = Parent;
	}

	vector<float> Distr = Weights;
	vector<vector<float>> trainpat = Data;
	vector<int> trainarg = Labels;

	vector<vector<float>> S_MIN;
	S_MIN.reserve(width);

	for(int d = 0; d < width; d++){

		//populate trainpat with d - single line of Data
		vector<float> sortline = trainpat[d];

		std::vector<int> IX;

		for (int i = 0; i < height; ++i)
			IX.push_back(i);

		std::sort(IX.begin(), IX.end(), index_cmp<std::vector<float>&>(sortline));

		std::vector<float> DS = sortline;
		std::sort(DS.begin(), DS.end());

		std::vector<int> TS;
		std::vector<float> DiS;
		 for(int i =0; i < (int)IX.size(); i++){
			 int idx = IX[i];
			 TS.push_back(trainarg[idx]);
			 DiS.push_back(Distr[idx]);
		 }
		 IX.clear();

		 std::vector<float> vPos(height);
		 std::vector<float> vNeg(height);

		 std::fill(vPos.begin(),vPos.end(),(float)0);
		 std::fill(vNeg.begin(),vNeg.end(),(float)0);

		 int i = 0;
		 int j = 0;

		 while(i < height){
			 int k = 0;
			 while(i+k < height && DS[i] == DS[i+k]){
				 if(TS[i+k] > 0)
					 vPos[j] += DiS[i+k];
				 else
					 vNeg[j] +=  DiS[i+k];
				 k++;
			 }
			 i += k;
			 j++;
		 }

		 vNeg.resize(j);
		 vPos.resize(j);
		 vNeg.shrink_to_fit();
		 vPos.shrink_to_fit();

		 std::vector<float> Error(j);
		 std::fill(Error.begin(),Error.end(),(float)0);
		 std::vector<float> InvError = Error;

		 std::vector<float> IPos = vPos;
		 std::vector<float> INeg = vNeg;


		 for(int i = 1; i< (int)IPos.size(); i++){
			 IPos[i] = IPos[i-1] + vPos[i];
			 INeg[i] = INeg[i-1] + vNeg[i];
		 }
		 
		 float Ntot = INeg.back();
		 float Ptot = IPos.back();

		 #pragma parallel
		 #pragma loop count min(128)
		 for(int i = 0; i < j; i++){
			 Error[i] = IPos[i] + Ntot - INeg[i];
			 InvError[i] = INeg[i] + Ptot - IPos[i];
		 }


		 int idx_of_err_min = min_element(Error.begin(),Error.end())-Error.begin();
		 int idx_of_inv_err_min = min_element(InvError.begin(),InvError.end())-InvError.begin();


		 vector<float> A_MIN;
		 A_MIN.reserve(3);
		  if(Error[idx_of_err_min] < InvError[idx_of_inv_err_min]) {
			A_MIN.push_back(Error[idx_of_err_min]);
			A_MIN.push_back((float)idx_of_err_min);
			A_MIN.push_back((float)-1);
			S_MIN.push_back(A_MIN);
		  }
		  else {
			A_MIN.push_back(InvError[idx_of_inv_err_min]);
			A_MIN.push_back((float)idx_of_inv_err_min);
			A_MIN.push_back((float)1);
			S_MIN.push_back(A_MIN);
		  }
		  A_MIN.clear();
	}

	int dim;
	//populate 
	std::vector<float>best_dim;

	//copy the first colum of S_MIN to best_dim
		for (int w = 0; w < width; w++) {
			best_dim.push_back(S_MIN[w][0]);
		}
   //find min_element
    std::vector<float>::iterator it;
	it = std::find(best_dim.begin(), best_dim.end(), *std::min_element(best_dim.begin(), best_dim.end()));

	dim = it-best_dim.begin();

	tree_node_left->dim = dim;
	tree_node_right->dim = dim;

	//TDS = line of dim; //aka height
	std::vector<float> TDS;
	TDS = Data[dim];
	sort(TDS.begin(),TDS.end());

	int IDS = (int)TDS.size();

	std::vector<float> DS(IDS);
    std::fill(DS.begin(),DS.end(),(float)0);

	int i = 0;
	int j = 0;

	while(i < IDS){
		int k = 0;
		while(i+k < IDS && TDS[i] == TDS[i+k]){
			DS[j] = TDS[i];
			k++;
		}
		i += k;
		j++;
	}

	DS.resize(j);
	DS.shrink_to_fit();

	std::vector<float> findMin;

	findMin.push_back(S_MIN[dim][1]+1); 
	findMin.push_back((float)DS.size()-1);

	float minVal = *std::min_element(findMin.begin(),findMin.end());

	float split = (DS[(int)S_MIN[dim][1]] + DS[(int)minVal]) / (float)2;
	
	float split_error = S_MIN[dim][0];

	tree_node_left->right_constraint = split;
	tree_node_right->left_constraint = split;

	get<0>(retVal) = tree_node_left;
	get<1>(retVal) = tree_node_right;
	get<2>(retVal) = split_error;

	return retVal;
}

tuple<vector<float>,vector<float>> Boosting::genWeights(tree_node* left, tree_node* right, vector<vector<float>> Data, vector<int> Labels, vector<float> Weights, int width, int height){

	tuple<std::vector<float>,std::vector<float>> retVal;

		std::vector<float> leftWeights;
		std::vector<float> rightWeights;
		std::vector<int> leftTemp(height);
		std::vector<int> rightTemp(height);


		leftTemp = calc_output(left,Data, width, height); //get<0>(output)
		rightTemp = calc_output(right,Data, width, height); //get<1>(output)

		std::vector<float> weightLabels(height);

		//Labels 
		std::vector<int> posLabels = Labels;
		std::vector<int> negLabels(height);
		std::transform (Labels.begin(), Labels.end(), negLabels.begin(), op_negative);

		std::vector<int> positive;
		std::vector<int> negative;

		//count positive left samples
		positive = findLabels(posLabels, leftTemp, height); 
		//count negative left samples
		negative = findLabels(negLabels, leftTemp, height); 

		std::transform(positive.begin(),positive.end(),Weights.begin(),weightLabels.begin(),std::multiplies<float>());
		leftWeights.push_back(std::accumulate(weightLabels.begin(),weightLabels.end(),(float)0,std::plus<float>()));

		std::transform(negative.begin(),negative.end(),Weights.begin(),weightLabels.begin(),std::multiplies<float>());
		leftWeights.push_back(std::accumulate(weightLabels.begin(),weightLabels.end(),(float)0,std::plus<float>()));

		//count positive right samples
		positive = findLabels(posLabels, rightTemp, height); 
		//count negative right  samples
		negative = findLabels(negLabels, rightTemp, height); 

		std::transform(positive.begin(),positive.end(),Weights.begin(),weightLabels.begin(),std::multiplies<float>());
		rightWeights.push_back(std::accumulate(weightLabels.begin(),weightLabels.end(),(float)0,std::plus<float>()));

		std::transform(negative.begin(),negative.end(),Weights.begin(),weightLabels.begin(),std::multiplies<float>());
		rightWeights.push_back(std::accumulate(weightLabels.begin(),weightLabels.end(),(float)0,std::plus<float>()));


		get<0>(retVal) = leftWeights;
		get<1>(retVal) = rightWeights;

		return retVal;
}

vector<int> Boosting::calc_output(tree_node* WeakLrn, vector<vector<float>> Data, int width, int height){
		std::vector<int> y(height);
	//fill vector with ones
	std::fill(y.begin(),y.end(),1);

	//check parent exists
	if(WeakLrn->parent != NULL)
	{
		std::vector<int> rec;
		rec = calc_output(WeakLrn->parent, Data, width, height);
		std::transform(rec.begin(),rec.end(),y.begin(),y.begin(),std::multiplies<int>());
		
	}
	
	//XData = line tree_node.dim of DATA
		std::vector<float> XData;
		XData = Data[WeakLrn->dim];

	if(WeakLrn->right_constraint != 0){
		//compare XData < WeakLrn.right_contrain
		std::vector<int>cmp;
		cmp = cmpData(XData, 0, (float)WeakLrn->right_constraint);
		std::transform(cmp.begin(),cmp.end(),y.begin(),y.begin(),std::multiplies<int>());
	}

	if(WeakLrn->left_constraint != 0){
		//compare XData > WeakLrn.left_contrain
		std::vector<int>cmp;
		cmp = cmpData(XData, 1, (float)WeakLrn->left_constraint);
		std::transform(cmp.begin(),cmp.end(),y.begin(),y.begin(),std::multiplies<int>());
	}

	return y;
}

std::vector<int> Boosting::cmpData(std::vector<float> XData, int condition, float value){

	std::vector<int> retVal(XData.size());
	switch(condition){
	case 0:
		#pragma loop count min(256)
		for(int i = 0; i < (int)XData.size(); i++)
		{
			if(XData[i] < value)
				retVal[i] = 1;
			else
				retVal[i] = 0;
		}
		break;

	case 1:
		#pragma loop count min(256)
		for(int i = 0; i < (int)XData.size(); i++)
		{
		   if(XData[i] > value)
				retVal[i] = 1;
			else
				retVal[i] = 0;
		}
		break;

	}

    return retVal;
}

vector<int> Boosting::findLabels(vector<int> Labels, vector<int> condition, int height){
	
	//initalise retVal
	std::vector<int> retVal;
	retVal.resize(height);
	fill (retVal.begin(),retVal.end(),(0));

	for(int i = 0; i < height; i++){
		if(Labels[i] == condition[i])
			retVal[i] = 1;
	}

	return retVal;
}

std::vector<int> Boosting::findLabels(std::vector<int> Labels, int condition, int height){
	
	//initalise retVal
	std::vector<int> retVal;
	retVal.resize(height);
	fill (retVal.begin(),retVal.end(),(0));

	for(int i = 0; i < height; i++){
		if(Labels[i] == condition)
			retVal[i] = 1;
	}

	return retVal;
}


vector<float> Boosting::Classify(vector<tree_node*> Learners, vector<float> Weights, vector <vector <float> > ControlData, int width, int height){

	vector<float> results(height);

	vector<vector<int>> lrn_out(Weights.size());

	tbb::parallel_for (int (0), (int)Weights.size(), [&](int i) {
	//for(int i = 0; i < Weights.size(); i++){
		lrn_out[i] = calc_output(Learners[i],ControlData,width,height);
		
	//}
	});
	for(int i = 0; i < Weights.size(); i++){
		std::transform(lrn_out[i].begin(), lrn_out[i].end(), lrn_out[i].begin(), bind2nd(std::multiplies<float>(), Weights[i]));  
		std::transform (results.begin(), results.end(), lrn_out[i].begin(), results.begin(), std::plus<float>());
	}

	return results;
}

vector<int> Boosting::sign(vector<float> input,  int width, int height){

	vector<int> results(height);
	#pragma loop count min(256)
	for(int i = 0; i < input.size(); i++){
		if(input[i] > 0 )
			results[i] = 1;
		else
			results[i] = -1;

		if(input[i] == 0)
			results[i] = 0;
	}

	return results;
}

vector<int> Boosting::sign4(vector<float> input1, vector<float> input2, vector<float> input3, vector<float> input4, vector<float> input5, vector<float> input6,  int width, int height, int* sign_nb, std::vector<int> bg){

	vector<int> results(height);
	#pragma loop count min(1200)
	for(int i = 0; i < input1.size(); i++){
		if(input1[i] > input2[i] && input1[i] > input3[i]  && input1[i] > input4[i] &&  input1[i] > input5[i] && input1[i] > input6[i]){
			results[i] = 1;
			if(bg[i] == 0)
				sign_nb[1]++;
		}
		else if(input2[i] > input1[i] && input2[i] > input3[i]  && input2[i] > input4[i] && input2[i] > input4[i] && input2[i] > input6[i] ){
			results[i] = 2;
			if(bg[i] == 0)
				sign_nb[2]++;
		}
		else if(input3[i] > input1[i] && input3[i] > input2[i]  && input3[i] > input4[i]  && input3[i] > input5[i] && input3[i] > input6[i]){
			results[i] = 3;
			if(bg[i] == 0)
				sign_nb[3]++;
		}
		else if (input4[i] > input1[i] && input4[i] > input2[i]  && input4[i] > input3[i] && input4[i] > input5[i] && input4[i] > input6[i] ){
			results[i] = 4;
			if(bg[i] == 0)
				sign_nb[4]++;
		}
		else if (input5[i] > input1[i] && input5[i] > input2[i]  && input5[i] > input3[i] && input5[i] > input4[i] && input5[i] > input6[i] ){
			results[i] = 5;
			if(bg[i] == 0)
				sign_nb[5]++;
		}
		else if (input6[i] > input1[i] && input6[i] > input2[i]  && input6[i] > input3[i] && input6[i] > input5[i] && input6[i] > input4[i] ){
			results[i] = 4;
			if(bg[i] == 0)
				sign_nb[6]++;
		}
		else
			results[i] = 0;
	}

	return results;
}