#ifndef Backseg_H
#define Backseg_H

#include <cv.h>
#include <highgui.h>
#undef min    
#undef max 
#include <../opencv2/gpu/gpu.hpp>
#include <tbb\parallel_for.h>

class Backseg
{
    private:
	CvMatND* matRGB;
	CvMatND* matRGBn;
	CvMatND* matSkw;

	std::vector<float> orginalMinValue;
	std::vector<float> orginalMaxValue;
	std::vector<float> orginalRangeValue;

	int desiredMin;
	int desiredMax;
	int desiredRange;

	public:
		Backseg();
		~Backseg();

	public:
		void LearnBackground(std::vector<std::vector<float>> Data);
		std::vector<int> RemoveBackground(std::vector<std::vector<float>> Data, int Threshold);
		void UpdateBackground(std::vector<std::vector<float>> Data, int Value);
    private:

};

#endif 