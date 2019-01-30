#include "FastImgSeg.h"
using namespace std;

//index for enforce connectivity
const int dx4[4] = {-1,  0,  1,  0};
const int dy4[4] = { 0, -1,  0,  1};

FastImgSeg::FastImgSeg(int w,int h, int nSegments)
{
	//contructor

	width=w;
	height=h;
	nSeg=nSegments;

	sourceImage=(unsigned char*)malloc(width*height*4*sizeof(unsigned char));

	segMask=(int*) malloc(width*height*sizeof(int));
	markedImg=(unsigned char*)malloc(width*height*4*sizeof(unsigned char));

	InitCUDA(width,height,nSegments,SLIC);

	bImgLoaded=false;
	bSegmented=false;
}

FastImgSeg::FastImgSeg()
{
}

FastImgSeg::~FastImgSeg()
{
	clearFastSeg();
}


void FastImgSeg::changeClusterNum(int nSegments)
{
	nSeg=nSegments;
}

void FastImgSeg::clearFastSeg()
{
	try
	{
	free(segMask);
	free(markedImg);
	TerminateCUDA();
	bImgLoaded=false;
	bSegmented=false;
	}
	catch (...)
	{
	}
}

void FastImgSeg::LoadImg(unsigned char* imgP)
{
    sourceImage=imgP;

	CUDALoadImg(sourceImage);
	bSegmented=false;
}

void FastImgSeg::DoSegmentation(SEGMETHOD eMethod, double weight)
{

		CudaSegmentation(nSeg,eMethod, weight);

		CopyMaskDeviceToHost(segMask,width,height);

		enforceConnectivity(segMask,width,height,nSeg);

		bSegmented=true;

}

// from original SLIC
void FastImgSeg::FindNext(const int* labels, int* nlabels, const int& height, const int& width, const int& h,	const int& w,
					const int&	lab,	int*	xvec, int* yvec, int& count)
{
	int oldlab = labels[h*width+w];
	for( int i = 0; i < 4; i++ ) {
		int y = h+dy4[i];int x = w+dx4[i];
		if((y < height && y >= 0) && (x < width && x >= 0) )
		{
			int ind = y*width+x;
			if(nlabels[ind] < 0 && labels[ind] == oldlab )
			{
				xvec[count] = x;
				yvec[count] = y;
				count++;
				nlabels[ind] = lab;
				FindNext(labels, nlabels, height, width, y, x, lab, xvec, yvec, count);
			}
		}
	}
}

void FastImgSeg::enforceConnectivity(int* maskBuffer,int width, int height, int nSeg)
{

	int sz = width*height;
	int* nlabels=(int*)malloc(sz*sizeof(int));
	memset(nlabels,-1,sz*sizeof(int));
	int* labels=maskBuffer;

	const int SUPSZ = sz/labels[(height-1)*width+(width-1)];
	//const int SUPSZ = sz/nSeg;
	//------------------
	// labeling
	//------------------
	int lab=0;
	int i=0;
	int adjlabel=0;//adjacent label
	int* xvec = (int*)malloc(sz*sizeof(int)); //worst case size
	int* yvec = (int*)malloc(sz*sizeof(int)); //worst case size

	for( int h = 0; h < height; h++ ) {
		for( int w = 0; w < width; w++ ) {
			if(nlabels[i] < 0)
			{
				nlabels[i] = lab;
				//-------------------------------------------------------
				// Quickly find an adjacent label for use later if needed
				//-------------------------------------------------------
				for( int n = 0; n < 4; n++ ){
					int x = w + dx4[n];
					int y = h + dy4[n];

					if( (x >= 0 && x < width) && (y >= 0 && y < height) )
					{
						int nindex = y*width + x;
						if(nlabels[nindex] >= 0) adjlabel = nlabels[nindex];
					}
				 }

				xvec[0] = w; yvec[0] = h;
				int count=1;
				FindNext(labels, nlabels, height, width, h, w, lab, xvec, yvec, count);
				//-------------------------------------------------------
				// If segment size is less then a limit, assign an
				// adjacent label found before, and decrement label count.
				//-------------------------------------------------------
				if(count <= (SUPSZ >> 2))
				{
  				       for( int c = 0; c < count; c++ ) {
						int ind = yvec[c]*width+xvec[c];
						nlabels[ind] = adjlabel;
					}
					lab--;
				}
				lab++;
			}
			i++;
		}
	   }

	//------------------
	//numlabels = lab;
	//------------------
	if(xvec) free(xvec);
	if(yvec) free(yvec);

	memcpy(labels,nlabels,sz*sizeof(int));
	if (nlabels)
	{
		free(nlabels);
	}
	
}