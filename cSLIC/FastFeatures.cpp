#include "FastFeatures.h"


using namespace std;

FastFeatures::FastFeatures(int width,int height,int nSegments)
	:		 FastImgSeg(width, height, nSegments) //init FastImgSeg
{

	Labels = (nSegments + (nSegments/25));//increase by 10%  
	lWidth = 4; //width is static

	// L, R,G,B (R,G,B are Sums), On/Off; //malloc
	avgStore=(float*) malloc(8*Labels*sizeof(float));
	varStore=(float*) malloc(7*Labels*sizeof(float));
	skw1Store=(float*) malloc(7*Labels*sizeof(float));
	skw2Store=(float*) malloc(7*Labels*sizeof(float));

	normImage=(float*)malloc(width*height*3*sizeof(float));
	grayImage=(float*)malloc(width*height*1*sizeof(float));
	edgeImage=(float*)malloc(width*height*1*sizeof(float));
}

void FastFeatures::Tool_GetMarkedImg()
{
	if (!bSegmented)
	{
		return;
	}

	memcpy(markedImg,sourceImage,width*height*4*sizeof(unsigned char));

	Concurrency::parallel_for (int (1), height-1, [&](int i) {
		for (int j=1;j<width-1;j++)
		{
			int mskIndex=i*width+j;
			if (segMask[mskIndex]!=segMask[mskIndex+1] 
			|| segMask[mskIndex]!=segMask[(i-1)*width+j]
			|| segMask[mskIndex]!=segMask[mskIndex-1]
			|| segMask[mskIndex]!=segMask[(i+1)*width+j])
			{ 
				//line colour
				markedImg[mskIndex*4]=0;
				markedImg[mskIndex*4+1]=0; 
				markedImg[mskIndex*4+2]=255;
			}
		}
	});
}

void FastFeatures::Tool_NormalizedRGB(){

	if (!bSegmented)
	{
		return;
	}
	Concurrency::parallel_for (int (1), height-1, [&](int i) {
	//for (int i=1;i<height-1;i++) {
		for (int j=1;j<width-1;j++)
		{
			int mskIndex=i*width+j;
            float denorm = (sourceImage[mskIndex*4+2] + sourceImage[mskIndex*4+1] + sourceImage[mskIndex*4+0]);
			if(denorm == 0)
			{
				normImage[mskIndex*3+2]  = sourceImage[mskIndex*4+2]; //r
				normImage[mskIndex*3+1]  = sourceImage[mskIndex*4+1]; //g
				normImage[mskIndex*3+0]  = sourceImage[mskIndex*4+0]; //b
			}
			else
			{
				normImage[mskIndex*3+2]  = sourceImage[mskIndex*4+2] / denorm; //r
				normImage[mskIndex*3+1]  = sourceImage[mskIndex*4+1] / denorm; //g
				normImage[mskIndex*3+0]  = sourceImage[mskIndex*4+0] / denorm; //b
			}
			normImage[mskIndex*1+0]  = (float)0.2989 * sourceImage[mskIndex*4+2] + (float) 0.5870 * sourceImage[mskIndex*4+1] + (float)0.1140 *  sourceImage[mskIndex*4+0];
		}
	//}
		});
}


void FastFeatures::Tool_GetAvgImg()
{
	if (!bSegmented)
	{
		return;
	}

	//init to 0 //clear old data
	memset(avgStore,0,8*Labels*sizeof(float));

	for (int i=1;i<height-1;i++) {
		for (int j=1;j<width-1;j++)
		{
			int mskIndex=i*width+j;
			int msk = segMask[mskIndex];

			//
			avgStore[msk*8+0]++; //pixel count for each label
			avgStore[msk*8+1] += sourceImage[mskIndex*4+2]; //r
			avgStore[msk*8+2] += sourceImage[mskIndex*4+1]; //g
			avgStore[msk*8+3] += sourceImage[mskIndex*4]; //b
			avgStore[msk*8+4] += normImage[mskIndex*3+2]; //rn
			avgStore[msk*8+5] += normImage[mskIndex*3+1]; //gn
			avgStore[msk*8+6] += normImage[mskIndex*3+0]; //bn
			avgStore[msk*8+7] += grayImage[mskIndex*1+0]; //i
		}

	}

	shrink = 0;
	//lets calculate avgs
	Concurrency::parallel_for (int (0), Labels, [&](int i) {
		if ( avgStore[i*8+0] > 0) // remove this when we have an acutal label count
		{
			avgStore[i*8+1] /= avgStore[i*8+0]; //r
			avgStore[i*8+2] /= avgStore[i*8+0]; //g
			avgStore[i*8+3] /= avgStore[i*8+0]; //b
			avgStore[i*8+4] /= avgStore[i*8+0]; //b
			avgStore[i*8+5] /= avgStore[i*8+0]; //b
			avgStore[i*8+6] /= avgStore[i*8+0]; //b
			avgStore[i*8+7] /= avgStore[i*8+0]; //b
		}
		else
		{
			shrink++;
		}
	});


}

void FastFeatures::Tool_GetStdImg(){

	if (!bSegmented)
	{
		return;
	}

	memset(varStore,0,7*Labels*sizeof(int));

	for (int i=1;i<height-1;i++) {
		for (int j=1;j<width-1;j++)
		{
			int mskIndex=i*width+j;
			int msk = segMask[mskIndex];

			//work out total diffrence 
			varStore[msk*7+0] += pow(double(sourceImage[mskIndex*4+2] - avgStore[i*8+1]), 2); //r
			varStore[msk*7+1] += pow(double(sourceImage[mskIndex*4+1] - avgStore[i*8+2]), 2); //g
			varStore[msk*7+2] += pow(double(sourceImage[mskIndex*4+0] - avgStore[i*8+3]), 2); //b
			varStore[msk*7+3] += pow(double(normImage[mskIndex*3+2] - avgStore[i*8+4]), 2); //nr
			varStore[msk*7+4] += pow(double(normImage[mskIndex*3+1] - avgStore[i*8+5]), 2); //ng
			varStore[msk*7+5] += pow(double(normImage[mskIndex*3+0] - avgStore[i*8+6]), 2); //nb
			varStore[msk*7+6] += pow(double(grayImage[mskIndex*1+0] - avgStore[i*8+7]), 2); //i
		}
	}

	//lets calculate variance
	Concurrency::parallel_for (int (0), Labels, [&](int i) {
		if ( avgStore[i*8+0] > 0) // remove this when we have an acutal label count
		{
			varStore[i*7+0] /= avgStore[i*8+0]; //r
			varStore[i*7+1] /= avgStore[i*8+0]; //g
			varStore[i*7+2] /= avgStore[i*8+0]; //b
			varStore[i*7+3] /= avgStore[i*8+0]; //b
			varStore[i*7+4] /= avgStore[i*8+0]; //b
			varStore[i*7+5] /= avgStore[i*8+0]; //b
			varStore[i*7+6] /= avgStore[i*8+0]; //i
		}
	});
}

void FastFeatures::Tool_GetSkeImg(){
	if (!bSegmented)
	{
		return;
	}

	memset(skw1Store,0,7*Labels*sizeof(int));
	memset(skw2Store,0,7*Labels*sizeof(int));

	//value minus mean
	for (int i=1;i<height-1;i++) {
		for (int j=1;j<width-1;j++)
		{
			int mskIndex=i*width+j;
			int msk = segMask[mskIndex];

			//substract the mean for the raw vlaue and cube, sum
			skw1Store[msk*7+0] += pow(double(sourceImage[mskIndex*4+2] - avgStore[i*8+1]), 3); //r
			skw1Store[msk*7+1] += pow(double(sourceImage[mskIndex*4+1] - avgStore[i*8+2]), 3); //g
			skw1Store[msk*7+2] += pow(double(sourceImage[mskIndex*4+0] - avgStore[i*8+3]), 3); //b
			skw1Store[msk*7+3] += pow(double(normImage[mskIndex*3+2]  - avgStore[i*8+4]), 3); //rn
			skw1Store[msk*7+4] += pow(double(normImage[mskIndex*3+1]  - avgStore[i*8+5]), 3); //gn
			skw1Store[msk*7+5] += pow(double(normImage[mskIndex*3+0]  - avgStore[i*8+6]), 3); //bn
			skw1Store[msk*7+6] += pow(double(grayImage[mskIndex*1+0]  - avgStore[i*8+7]), 3); //bn
		}
	}

	//n - 1 * (stddev)3
	Concurrency::parallel_for (int (0), Labels, [&](int i) {
		if ( avgStore[i*8+0] > 0) // remove this when we have an acutal label count
		{
			skw2Store[i*7+0] = (avgStore[i*8+0] - 1) * pow(sqrt(double(varStore[i*7+0])),3); //r
			skw2Store[i*7+1] = (avgStore[i*8+0] - 1) * pow(sqrt(double(varStore[i*7+1])),3); //g
			skw2Store[i*7+2] = (avgStore[i*8+0] - 1) * pow(sqrt(double(varStore[i*7+2])),3); //b
			skw2Store[i*7+3] = (avgStore[i*8+0] - 1) * pow(sqrt(double(varStore[i*7+3])),3); //rn
			skw2Store[i*7+4] = (avgStore[i*8+0] - 1) * pow(sqrt(double(varStore[i*7+4])),3); //bn
			skw2Store[i*7+5] = (avgStore[i*8+0] - 1) * pow(sqrt(double(varStore[i*7+5])),3); //gn
			skw2Store[i*7+6] = (avgStore[i*8+0] - 1) * pow(sqrt(double(varStore[i*7+6])),3); //gn
		}
	});

	// skw1 = skw1 / skw2 

	Concurrency::parallel_for (int (0), Labels, [&](int i) {
		if ( skw2Store[i*7+0] > 0) // remove this when we have an acutal label count
			skw1Store[i*7+0]  /= skw2Store[i*7+0] ; //r
		if ( skw2Store[i*7+1] > 0)
			skw1Store[i*7+1] /= skw2Store[i*7+1]; //g
		if ( skw2Store[i*7+2] > 0)
			skw1Store[i*7+2] /= skw2Store[i*7+2]; //b
		if ( skw2Store[i*7+3] > 0)
			skw1Store[i*7+3] /= skw2Store[i*7+3]; //rn
		if ( skw2Store[i*7+4] > 0)
			skw1Store[i*7+4] /= skw2Store[i*7+4]; //gn
		if ( skw2Store[i*7+5] > 0)
			skw1Store[i*7+5] /= skw2Store[i*7+5]; //bn
	   if ( skw2Store[i*7+6] > 0)
			skw1Store[i*7+6] /= skw2Store[i*7+6]; //i
	});
}

void FastFeatures::Tool_GetEdgeImg(){
	if (!bSegmented)
	{
		return;
	}

	int          GX[3][3];
    int          GY[3][3];
    /* 3x3 GX Sobel mask.  Ref: www.cee.hw.ac.uk/hipr/html/sobel.html */
    GX[0][0] = -1; GX[0][1] = 0; GX[0][2] = 1;
    GX[1][0] = -2; GX[1][1] = 0; GX[1][2] = 2;
    GX[2][0] = -1; GX[2][1] = 0; GX[2][2] = 1;

    /* 3x3 GY Sobel mask.  Ref: www.cee.hw.ac.uk/hipr/html/sobel.html */
    GY[0][0] =  1; GY[0][1] =  2; GY[0][2] =  1;
    GY[1][0] =  0; GY[1][1] =  0; GY[1][2] =  0;
    GY[2][0] = -1; GY[2][1] = -2; GY[2][2] = -1;




}


FastFeatures::~FastFeatures()
{
	try
	{
		free(avgStore);
	}
	catch (...)
	{
	}
}

void FastFeatures::Tool_WriteMask2File(char* outFileName, bool writeBinary)
{
	
	if (!bSegmented)
	{
		return;
	}
	
	if (writeBinary)
	{
		ofstream outf;
		outf.open(outFileName, ios::binary);

		outf.write(reinterpret_cast<char*>(&width),sizeof(width));
		outf.write(reinterpret_cast<char*>(&height),sizeof(height));

		for (int i=0;i<height;i++)
		{
			for (int j=0;j<width;j++)
			{
				int mskIndex=i*width+j;
				int idx=segMask[mskIndex];
				outf.write(reinterpret_cast<char*>(&idx),sizeof(idx));
			}
		}
		outf.close();
	}
	else
	{
		ofstream outf;

		outf.open(outFileName);

		for (int i=0;i<height;i++)
		{
			for (int j=0;j<width;j++)
			{
				int mskIndex=i*width+j;
				int idx=segMask[mskIndex];
				outf<<idx<<',';
			}
			outf<<'\n';
		}
		outf.close();
	}
}