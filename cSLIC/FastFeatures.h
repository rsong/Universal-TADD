#ifndef FastFeatures_H
#define FastFeatures_H

#include "FastImgSeg.h"

class FastFeatures: public FastImgSeg
{
    public:
			int lWidth;
			int Labels;
			float* avgStore;
			float* varStore;
			float* skw1Store;
			float* skw2Store;
			int shrink;
			float* normImage;
			float* grayImage;
			float* edgeImage;

	public:
		FastFeatures(int width,int height,int nSegments);
		~FastFeatures();

		void Tool_NormalizedRGB();
		void Tool_GetAvgImg();
		void Tool_GetStdImg();
		void Tool_GetSkeImg();
		void Tool_GetEdgeImg();
		void Tool_GetMarkedImg();
		void Tool_WriteMask2File(char* outFileName, bool writeBinary);

	private:
		int CountLabels();
};

#endif 