#include <cv.h>
#include <highgui.h>

void evaluateGLDMPuzzle(IplImage *img, int cellRows, int cellCols){
	// NOTE: slightly quick and dirty implementation -- should take care, if it is kept working
	int xSquares = (img->width % cellCols == 0 ? img->width / cellCols : img->width / cellCols + 1);
	int ySquares = (img->height % cellRows == 0 ? img->height / cellRows : img->height / cellRows + 1);

	vector<CvRect> rectangles;
	splitImageIntoPuzzle(img, cellRows, cellCols, rectangles);

	if(img->nChannels == 3)
		getGrayscaleInput(img);

	// preparation of the output image
//		if (puzzleResult && (puzzleResult->width != cellCols * xSquares || puzzleResult->height != cellRows * ySquares))
	if (puzzleResult && (puzzleResult->width != gldm->width || puzzleResult->height != gldm->height))
	{
		cvReleaseImage(&puzzleResult);
		puzzleResult = 0;
	}

	if (!puzzleResult)
		puzzleResult = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 1);
//			puzzleResult = cvCreateImage(cvSize(cellCols * xSquares, cellRows * ySquares), IPL_DEPTH_8U, 1);

	cvSetZero(puzzleResult);

	IplImage *im = (img->nChannels == 1 ? img : grayscaleInput);
	// this is the the buffer that will be written with the actual block being processed -- for all blocks!
	IplImage *block = cvCreateImage(cvSize(cellCols, cellRows), IPL_DEPTH_8U, 1);

	for (unsigned int i = 0; i < rectangles.size(); ++i)
	{

	//for (int i = 0; i < xSquares; ++i)
	//{
	//	for (int j = 0; j < ySquares; ++j)
	//	{
		int w = rectangles[i].width;
		int h = rectangles[i].height;

//			int h = min(cellRows, im->height - j * cellRows);

		//cvSetImageROI(im,cvRect(i * cellCols, j * cellRows, w, h));
		cvSetImageROI(im, rectangles[i]);

		if (w < cellCols || h < cellRows)
		// selection should select a ROI on the block as well if the part of the
		// image we should analyse is smaller than the normal block, due to border effects.
		cvSetImageROI(block, cvRect(0, 0, w, h));

		// the block is copied into the buffer
		cvCopy(im, block);
		cvResetImageROI(im);
		cvResetImageROI(block);

		// evaluation of the GLDM on the single block
		evaluateGLDM(block, 1, H);

		// DEBUG: fill the gldm with a value that represents the feature value
		fillGLDMWithFeatureValue(ENTROPY);
		// this is used to copy the gldm into the gldmresult buffer
		gldmOriginal();

		/*
		// DEBUG: this should be commented as long as the the filling with the feature value is active!
		// further algorithms on the gldm
		gldmBinarization(cvGetSize(block));
		*/

		// copy of the GLDM result into the appropriate region of the whole puzzle image
//			cvSetImageROI(puzzleResult, cvRect(i * cellCols, j * cellRows, cellCols, cellRows));
		cvSetImageROI(puzzleResult, rectangles[i]);
		//cvConvertScale(GLDMResult, puzzleResult, static_cast<double> (GLDMResult->width) / cellRows);
		cvResize(GLDMResult, puzzleResult, CV_INTER_NN);
		cvResetImageROI(puzzleResult);
	}

	cvReleaseImage(&block);
	return;
}

void splitImageIntoPuzzle(IplImage *img, int cellRows, int cellCols, vector<CvRect> &elements){
	// this method splits otuputs a vector of elements, each one corresponding to a
	// image puzzle element
	int xSquares = (img->width % cellCols == 0 ? img->width / cellCols : img->width / cellCols + 1);
	int ySquares = (img->height % cellRows == 0 ? img->height / cellRows : img->height / cellRows + 1);

	for (int i = 0; i < xSquares; ++i)
	{
		for (int j = 0; j < ySquares; ++j)
		{
			int w = min(cellCols, img->width - i*cellCols);
			int h = min(cellRows, img->height - j * cellRows);
			elements.push_back(cvRect(i * cellCols, j * cellRows, w, h));
		}
	}

	return;
}

void getGrayscaleInput(IplImage *img){
	if (!img)
		return;

	if (grayscaleInput && (grayscaleInput->width != img->width || grayscaleInput->height != img->height))
	{
		cvReleaseImage(&grayscaleInput);
		grayscaleInput = 0;
	}

	if (!grayscaleInput)
	{
		grayscaleInput = cvCreateImage(cvSize(img->width, img->height), img->depth, 1);
	}

	if (img->roi)
		cvSetImageROI(grayscaleInput, cvRect(img->roi->xOffset, img->roi->yOffset,
						img->roi->width, img->roi->height));

	cvCvtColor(img, grayscaleInput, CV_RGB2GRAY);

	return;
}

void evaluateGLDM(IplImage *img, int d, enum GLDMAngle angle){
	const int gldmDim = 256;

	// does not make sense to evaluate the GLDM for depths higher than 8-bit, since this
	// would result in really huge output images! Also, 1-channel input image is required
	// Finally, if output image is provided, does not hadle the case when it is smaller than required
	if (!img || img->depth != IPL_DEPTH_8U)
		return;

	if (img->nChannels == 3)
		getGrayscaleInput(img);

	IplImage *im = img->nChannels == 1 ? img : grayscaleInput;

	// Should now allocate the destination image, which is, by definition,
	// a square image whose dimension is the number of grey levels.
	// If the dimensions are wrong, I should free it and allocate it again
	if (GLDMResult && (GLDMResult->width < gldmDim || GLDMResult->height < gldmDim))
	{
		cvReleaseImage(&GLDMResult);
		GLDMResult = 0;
	}

	if (!GLDMResult)
		GLDMResult = cvCreateImage(cvSize(gldmDim, gldmDim), IPL_DEPTH_8U, 1);

	// this is a private data member; it is allocated only once. This buffer cannot be shared
	// among other algorithms because it has weird dimensions
	if (!gldm)
		// floating-point image because each pixel can potentially store a very hight number
		gldm = cvCreateImage(cvSize(gldmDim, gldmDim), IPL_DEPTH_32F, 1);

	cvSetZero(gldm);

	// determine which are the starting coordinates of i -- j is then determined
	int iXStart = (angle == LD ? d : 0);
	int iXStop = (angle == H || angle == RD ? img->width - d : img->width);
	int iYStart = 0;
	int iYStop = (angle == H ? img->height : img->height - d);

	// roi management, used when working with puzzle
	if (im->roi)
	{
		iXStart += im->roi->xOffset;
		iXStop -= (im->width - (im->roi->xOffset + im->roi->width));
		iYStart += im->roi->yOffset;
		iYStop -= (im->height - (im->roi->yOffset + im->roi->height));

		// the following condition happens when the puzzle parts are smaller than d
		if (iXStart > iXStop || iYStart > iYStop)
		{
			cout << "GLDM not evaluated\n";
			return;
		}
	}

	// displacements of j WRT i
	int jXDispl;
	if (angle == V)
		jXDispl = 0;
	else if (angle == LD)
		jXDispl = -d;
	else
		jXDispl = d;

	int jYDispl = (angle == H ? 0 : d);

	// note the ugly casts I should perform due to the imageData pointer type!
	unsigned char *src = reinterpret_cast<unsigned char *> (im->imageData);

	float *dst = reinterpret_cast<float *> (gldm->imageData);

	// buffers
	unsigned char iGreylevel, jGreylevel;

	for (int x = iXStart; x < iXStop; ++x)
	{
		for (int y = iYStart; y < iYStop; ++y)
		{
			// I think this will be easy to optimize for the compiler
			// (i.e. it shouldn't be a problem to use the two buffers]
			iGreylevel = src[x + y * im->widthStep];
			jGreylevel = src[x + jXDispl + (y + jYDispl) * im->widthStep];

			// TODO: the following may be more efficient ?
			//++dst[iGreylevel + jGreylevel * (gldm->widthStep / sizeof(float))];

			float *curRow = reinterpret_cast<float *> (gldm->imageData + jGreylevel * gldm->widthStep);
			++curRow[iGreylevel];
		}
	}

	// DEBUG -- image closure, to cope with camera 1, that provides a trellish-shaped gldm
	if (!gldmCopyFloat)
		gldmCopyFloat = cvCreateImage(cvGetSize(gldm), gldm->depth, gldm->nChannels);
	cvDilate(gldm, gldmCopyFloat, NULL, 3);
	cvErode(gldmCopyFloat, gldm, NULL, 3);

	return;
}


float fillGLDMWithFeatureValue(enum GLDMFeat feature, IplImage *img){
	// Fill the whole gldm with a pixel colour related to the feature value
	// easy feedback to understand features (mainly for debug)
	static const float RESCALING[] = { 3000, 3000, 10, 500, 60 };
	float maxFeat = RESCALING[feature];

	// ok for contrast
	// static const float MAX_FEAT = 3000;

	// ok for homogeneity
	// static const float MAX_FEAT = 10;

	// ok for energy
	// static const float MAX_FEAT = 500;
	// energy calculated on the whole image
//		static const float MAX_FEAT = 3000000;

	// ok for entropy
	// static const float MAX_FEAT = 15;

	float featureValue = evaluateGLDMFeature(feature, img);
	float featValueRescaled = featureValue / maxFeat;
	for (int i = 0; i < gldm->height; ++i)
	{
		float *curRow = reinterpret_cast<float *> (gldm->imageData + i * gldm->widthStep);
		for (int j = 0; j < gldm->width; ++j)
		{
			curRow[j] = featValueRescaled;
		}
	}

	return featureValue;
}


float evaluateGLDMFeature(enum GLDMFeat feature, IplImage *img){
	// consider the gldm if no image is specified (and img is therefore == 0)
	IplImage *image = (img ? img : gldm);

	float sum = 0.0;

	for (int i = 0; i < image->height; ++i)
	{
		float *curRow = reinterpret_cast<float *> (image->imageData + i * image->widthStep);
		for (int j = 0; j < image->width; ++j)
		{
			switch (feature)
			{
			case CONTRAST:
				sum += (j-i) * (j-i) * curRow[j];
				break;
			// this is a slightly different version of contrast: substitute the square with the absolute value,
			// not to give too high value to distant components -- suitable for off-peak component analysis
			case LINEAR_CONTRAST:
				sum += abs(j-i) * curRow[j];
				break;
			case HOMOGENEITY:
				sum += curRow[j] / (1 + ((j-i) * (j-i)));
				break;
			case ENERGY:
				sum += curRow[j] * curRow[j];
				break;
			case ENTROPY:
				if (curRow[j] > 1.5)
					sum += curRow[j] * log(curRow[j]);
				break;
			default:
				break;
			}
		}
	}

	return sum;
}

void gldmOriginal(void){
		cvConvertScale(gldm, GLDMResult);

		return;
}