#include "Backseg.h"

Backseg::Backseg()
{
	/* Matrix */ 
	int size[] = { 282, 282, 282 };
    matRGB = cvCreateMatND(3, size, CV_8U);
	matRGBn = cvCreateMatND(3, size, CV_8U);
	matSkw = cvCreateMatND(3, size, CV_8U);
	cvSetZero(matRGB);
	cvSetZero(matRGBn);
	cvSetZero(matSkw);

	
	orginalMinValue.resize(9);
	orginalMaxValue.resize(9);
	orginalRangeValue.resize(9);

	orginalMinValue[0] = 0; //R
	orginalMinValue[1] = 0; //G 
	orginalMinValue[2] = 0; //B
	orginalMinValue[3] = 0; //Rn
	orginalMinValue[4] = 0; //Gn 
	orginalMinValue[5] = 0; //Bn
	orginalMinValue[6] = -10; //Rskw
	orginalMinValue[7] = -10; //Gskw 
	orginalMinValue[8] = -10; //Bskw


	orginalMaxValue[0] = 255; //R
	orginalMaxValue[1] = 255; //G 
	orginalMaxValue[2] = 255; //B
	orginalMaxValue[3] = 255; //Rn
	orginalMaxValue[4] = 255; //Gn 
	orginalMaxValue[5] = 255; //Bn
	orginalMaxValue[6] = 10; //Rskw
	orginalMaxValue[7] = 10; //Gskw 
	orginalMaxValue[8] = 10; //Bskw

	orginalRangeValue[0] = orginalMaxValue[0] - orginalMinValue[0];
	orginalRangeValue[1] = orginalMaxValue[1] - orginalMinValue[1];
	orginalRangeValue[2] = orginalMaxValue[2] - orginalMinValue[2];
	orginalRangeValue[3] = orginalMaxValue[3] - orginalMinValue[3];
	orginalRangeValue[4] = orginalMaxValue[4] - orginalMinValue[4];
	orginalRangeValue[5] = orginalMaxValue[5] - orginalMinValue[5];
	orginalRangeValue[6] = orginalMaxValue[6] - orginalMinValue[6];
	orginalRangeValue[7] = orginalMaxValue[7] - orginalMinValue[7];
	orginalRangeValue[8] = orginalMaxValue[8] - orginalMinValue[8];

	desiredMin = 13;
	desiredMax = 255;
	desiredRange = desiredMax - desiredMin;

}

//pass vector of R,G,B,Rn,Gn,Bn,SkwR,SkwG,SkeB
void Backseg::LearnBackground(std::vector<std::vector<float>> Data){

	std::vector<float> Channel = Data[0];

	//go through the size of Labels
	//tbb::parallel_for(int (0), (int)Channel.size(), [&](int i) {
	for(int i = 0; i < Channel.size(); i++){
			// R, G , B
			//Get Location
		    float R = Data[0][i];
			float G = Data[1][i];
			float B = Data[2][i];

			//normalisze to 255 grid
			int gR = desiredRange * (R - orginalMinValue[0]) / orginalRangeValue[0] + desiredMin ;
			int gG = desiredRange * (G - orginalMinValue[1]) / orginalRangeValue[1] + desiredMin ;
			int gB = desiredRange * (B - orginalMinValue[2]) / orginalRangeValue[2] + desiredMin ;
            
			
			//box points 
			/*int boxsize = 2;
			
			gR = gR - ((boxsize-1)/2);
			gG = gG - ((boxsize-1)/2);
			gB = gB - ((boxsize-1)/2);
         
			
             for(int b = 0; b < boxsize; b++){
				for(int c = 0; c < boxsize; c++){
					for(int d = 0; d < boxsize; d++) {
						matRGB->data.ptr[(gR + c)*matRGB->dim[0].step + (gG+b)*matRGB->dim[0].size + (gB + d)] = 1;
					}
				}
			 }*/

			 matRGB->data.ptr[(gR)*matRGB->dim[0].step + (gG)*matRGB->dim[0].size + (gB)] = 1;

			 /*
			
			//Update RGB Hit Table 

			matRGB->data.ptr[gR*matRGB->dim[0].step + gG*matRGB->dim[0].size + gB] = 1;

			matRGB->data.ptr[(gR - 1)*matRGB->dim[0].step + (gG - 1 )*matRGB->dim[0].size + (gB - 1)] = 1;
			matRGB->data.ptr[(gR - 1)*matRGB->dim[0].step + (gG - 1 )*matRGB->dim[0].size + (gB)] = 1;
			matRGB->data.ptr[(gR - 1)*matRGB->dim[0].step + (gG - 1 )*matRGB->dim[0].size + (gB + 1)] = 1;

			matRGB->data.ptr[(gR)*matRGB->dim[0].step + (gG - 1 )*matRGB->dim[0].size + (gB - 1)] = 1;
			matRGB->data.ptr[(gR)*matRGB->dim[0].step + (gG - 1 )*matRGB->dim[0].size + (gB)] = 1;
			matRGB->data.ptr[(gR)*matRGB->dim[0].step + (gG - 1 )*matRGB->dim[0].size + (gB +1)] = 1;
				
			matRGB->data.ptr[(gR + 1)*matRGB->dim[0].step + (gG - 1 )*matRGB->dim[0].size + (gB - 1)] = 1;
			matRGB->data.ptr[(gR + 1)*matRGB->dim[0].step + (gG - 1 )*matRGB->dim[0].size + (gB)] = 1;
			matRGB->data.ptr[(gR + 1)*matRGB->dim[0].step + (gG - 1 )*matRGB->dim[0].size + (gB + 1)] = 1;

			///

			matRGB->data.ptr[(gR - 1)*matRGB->dim[0].step + (gG)*matRGB->dim[0].size + (gB - 1)] = 1;
			matRGB->data.ptr[(gR - 1)*matRGB->dim[0].step + (gG)*matRGB->dim[0].size + (gB)] = 1;
			matRGB->data.ptr[(gR - 1)*matRGB->dim[0].step + (gG)*matRGB->dim[0].size + (gB + 1)] = 1;

			matRGB->data.ptr[(gR)*matRGB->dim[0].step + (gG)*matRGB->dim[0].size + (gB - 1)] = 1;
			matRGB->data.ptr[(gR)*matRGB->dim[0].step + (gG)*matRGB->dim[0].size + (gB)] = 1;
			matRGB->data.ptr[(gR)*matRGB->dim[0].step + (gG)*matRGB->dim[0].size + (gB +1)] = 1;
				
			matRGB->data.ptr[(gR + 1)*matRGB->dim[0].step + (gG)*matRGB->dim[0].size + (gB - 1)] = 1;
			matRGB->data.ptr[(gR + 1)*matRGB->dim[0].step + (gG)*matRGB->dim[0].size + (gB)] = 1;
			matRGB->data.ptr[(gR + 1)*matRGB->dim[0].step + (gG)*matRGB->dim[0].size + (gB + 1)] = 1;

			////

			matRGB->data.ptr[(gR - 1)*matRGB->dim[0].step + (gG + 1 )*matRGB->dim[0].size + (gB - 1)] = 1;
			matRGB->data.ptr[(gR - 1)*matRGB->dim[0].step + (gG + 1 )*matRGB->dim[0].size + (gB)] = 1;
			matRGB->data.ptr[(gR - 1)*matRGB->dim[0].step + (gG + 1 )*matRGB->dim[0].size + (gB + 1)] = 1;

			matRGB->data.ptr[(gR)*matRGB->dim[0].step + (gG + 1 )*matRGB->dim[0].size + (gB - 1)] = 1;
			matRGB->data.ptr[(gR)*matRGB->dim[0].step + (gG + 1 )*matRGB->dim[0].size + (gB)] = 1;
			matRGB->data.ptr[(gR)*matRGB->dim[0].step + (gG + 1 )*matRGB->dim[0].size + (gB +1)] = 1;
				
			matRGB->data.ptr[(gR + 1)*matRGB->dim[0].step + (gG + 1 )*matRGB->dim[0].size + (gB - 1)] = 1;
			matRGB->data.ptr[(gR + 1)*matRGB->dim[0].step + (gG + 1 )*matRGB->dim[0].size + (gB)] = 1;
			matRGB->data.ptr[(gR + 1)*matRGB->dim[0].step + (gG + 1 )*matRGB->dim[0].size + (gB + 1)] = 1;
			

			// nR, nG , nB
			//Get Location
			float Rn = Data[3][i];
			float Gn = Data[4][i];
			float Bn = Data[5][i];

			//normalisze to 255 grid
			int gRn = desiredRange * (Rn - orginalMinValue[3]) / orginalRangeValue[3] + desiredMin ;
			int gGn = desiredRange * (Gn - orginalMinValue[4]) / orginalRangeValue[4] + desiredMin ;
			int gBn = desiredRange * (Bn - orginalMinValue[5]) / orginalRangeValue[5] + desiredMin ;

			//Update nRGB Hit Table 
			matRGBn->data.ptr[gRn*matRGBn->dim[0].step + gGn*matRGBn->dim[0].size + gBn] = 1;

			//skwR, skwG, skwB  
			//Get Location
			float skwR = Data[6][i];
			float skwG = Data[7][i];
			float skwB = Data[8][i];

			//normalisze to 255 grid
			int gSkwR = desiredRange * (skwR - orginalMinValue[6]) / orginalRangeValue[6] + desiredMin ;
			int gSkwG = desiredRange * (skwG - orginalMinValue[7]) / orginalRangeValue[7] + desiredMin ;
			int gSkwB = desiredRange * (skwB - orginalMinValue[8]) / orginalRangeValue[8] + desiredMin ;

			//Update Skw Hit Table 
			matSkw->data.ptr[gSkwR*matSkw->dim[0].step + gSkwG*matSkw->dim[0].size + gSkwB] = 1;
			*/
	}
	//}); 

int i =0;

}

//if more then two return true, postive backgorund hit
std::vector<int> Backseg::RemoveBackground(std::vector<std::vector<float>> Data, int Threshold){

	std::vector<float> Channel = Data[0];;

	std::vector<int> match(Channel.size());
	std::fill(match.begin(),match.end(),0); 

	//go through the size of Labels
	//tbb::parallel_for(int (0), (int)Channel.size(), [&](int i) {
	for(int i = 0; i < Channel.size(); i++){
			// R, G , B
			//Get Location
		    float R = Data[0][i];
			float G = Data[1][i];
			float B = Data[2][i];

			//normalisze to 255 grid
			int gR = desiredRange * (R - orginalMinValue[0]) / orginalRangeValue[0] + desiredMin ;
			int gG = desiredRange * (G - orginalMinValue[1]) / orginalRangeValue[1] + desiredMin ;
			int gB = desiredRange * (B - orginalMinValue[2]) / orginalRangeValue[2] + desiredMin ;

			//Check hittable
			if( matRGB->data.ptr[gR*matRGB->dim[0].step + gG*matRGB->dim[0].size + gB] >= Threshold)
				match[i]++;
			/*
			// nR, nG , nB
			//Get Location
			float Rn = Data[3][i];
			float Gn = Data[4][i];
			float Bn = Data[5][i];

			//normalisze to 255 grid
			int gRn = desiredRange * (Rn - orginalMinValue[3]) / orginalRangeValue[3] + desiredMin ;
			int gGn = desiredRange * (Gn - orginalMinValue[4]) / orginalRangeValue[4] + desiredMin ;
			int gBn = desiredRange * (Bn - orginalMinValue[5]) / orginalRangeValue[5] + desiredMin ;

			//Check hittable
			if(matRGBn->data.ptr[gRn*matRGBn->dim[0].step + gGn*matRGBn->dim[0].size + gBn] >= Threshold)
				match[i]++;

			
			//skwR, skwG, skwB  
			//Get Location
			float skwR = Data[6][i];
			float skwG = Data[7][i];
			float skwB = Data[8][i];

			//normalisze to 255 grid
			int gSkwR = desiredRange * (skwR - orginalMinValue[6]) / orginalRangeValue[6] + desiredMin ;
			int gSkwG = desiredRange * (skwG - orginalMinValue[7]) / orginalRangeValue[7] + desiredMin ;
			int gSkwB = desiredRange * (skwB - orginalMinValue[8]) / orginalRangeValue[8] + desiredMin ;

			//Check hittable
			if(matSkw->data.ptr[gSkwR*matSkw->dim[0].step + gSkwG*matSkw->dim[0].size + gSkwB] >= Threshold)
				match[i]++;
				*/
	//});
	}
	std::vector<int> background(match.size());

	tbb::parallel_for(int (0), (int)match.size(), [&](int i) {
		if(match[i] >= 1)
			background[i] = 1;
		else
			background[i] = 0;
	});
	return background;
}


//method to add one block to model
void Backseg::UpdateBackground(std::vector<std::vector<float>> Data, int Value){

	std::vector<float> Channel = Data[0];

	std::vector<int> match(Channel.size());
	std::fill(match.begin(),match.end(),0); 


	for(int i = 0; i < Channel.size(); i++){
			// R, G , B
			//Get Location
		    float R = Data[0][i];
			float G = Data[1][i];
			float B = Data[2][i];

			//normalisze to 255 grid
			int gR = desiredRange * (R - orginalMinValue[0]) / orginalRangeValue[0] + desiredMin ;
			int gG = desiredRange * (G - orginalMinValue[1]) / orginalRangeValue[1] + desiredMin ;
			int gB = desiredRange * (B - orginalMinValue[2]) / orginalRangeValue[2] + desiredMin ;
            
			int boxsize = 3;
			//box points 
			if(Value ==1){
				boxsize = 17;
			}
			if(Value == 0){
				boxsize = 7;
			}
			
			gR = gR - ((boxsize-1)/2);
			gG = gG - ((boxsize-1)/2);
			gB = gB - ((boxsize-1)/2);
         
			
             for(int b = 0; b < boxsize; b++){
				for(int c = 0; c < boxsize; c++){
					for(int d = 0; d < boxsize; d++) {
						matRGB->data.ptr[(gR + c)*matRGB->dim[0].step + (gG+b)*matRGB->dim[0].size + (gB + d)] = Value;
					}
				}
			 }
	}

}

Backseg::~Backseg()
{
	cvReleaseMatND(&matRGB);
	cvReleaseMatND(&matRGBn);
	cvReleaseMatND(&matSkw);
}