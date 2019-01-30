#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <malloc.h>
#include <crtdbg.h>

#ifndef __CUDA_SUPERPIXELSEG__
#define __CUDA_SUPERPIXELSEG__

#include <stdio.h>
#include <string.h>
#include <fstream>
#include <time.h>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <ppl.h>

#include "../gSLIC/cudaSegEngine.cuh"
#include "../gSLIC/cudaUtil.cuh"
#include "../gSLIC/cudaSegSLIC.cuh"

class FastImgSeg {

public:
	unsigned char* sourceImage;
	unsigned char* markedImg;
	int* segMask;


 protected:

	int width;
	int height;
	int nSeg;

	bool bSegmented;
	bool bImgLoaded;
	bool bMaskGot;

	

public:
	FastImgSeg();
	FastImgSeg(int width,int height,int nSegments);
	~FastImgSeg();

	void initializeFastSeg(int width,int height,int nSegments);
	void clearFastSeg();
	void changeClusterNum(int nSegments);

	void LoadImg(unsigned char* imgP);
	void DoSegmentation(SEGMETHOD eMethod, double weight);

private:
	void enforceConnectivity(int* maskBuffer,int width, int height, int nSeg);
	void FindNext(const int* labels, int* nlabels, const int& height, const int& width, const int& h,	const int& w,
					const int&	lab,	int*	xvec, int* yvec, int& count);

};

#endif
