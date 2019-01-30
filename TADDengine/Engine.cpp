#include "Engine.h"

TADDengine::TADDengine(int wid, int heig){
	///
	///Constructor, sets the image width and height for the system and calls initLoad()
	width = wid;
	height = heig;

	FEATSTEP = 28;

	char msg[100];
	sprintf(msg,"Wid: %d Hgt: %d");

	//dbgIplImg = cvCreateImage(cvSize(width, height),IPL_DEPTH_8U,3);
	debugimg = 0;(unsigned char *)dbgIplImg->imageData;

	//due to loading quick hack
	initLoad();
}

TADDengine::TADDengine(){

}

void TADDengine::initLoad(bool preserve){
	///
	/// Sets up most of the variables used by the system, as well as setting up memory. 
	/// If preserve is true, this means that the training data is already populated and 
	/// should not be replaced.

    //create gSlic object for superpixel segmentation and features
	sPixels = 9200;
	//sPixels = height*width/100; // Segmentation seems to work best at 100 pixel average region sizes.

	segMethod = SEGMETHOD::XYZ_SLIC;
	segWeight = 0.1;

	mySeg = new FastFeatures(width,height,sPixels); 

	//segmentation buffers
	imgBuffer=(unsigned char*)malloc(width*height*sizeof(unsigned char)*4);
	memset(imgBuffer,0,width*height*sizeof(unsigned char)*4);
	imgBuffer4 = cvCreateImage( cvSize( width,height ), IPL_DEPTH_8U, 4);
	zeros = cvCreateMat(height,width, CV_8UC1);

	//classifer object
	//myClass = new FastKNN();
	
	//Adaboost
	Adaboost = new Boosting(100); //100 is iterations
	//overlay

	Background = new Backseg();
	//backgroud buffer
	threshBuffer=(unsigned char*)malloc(width*height*sizeof(unsigned char));

	//cvOverlay
	cvOverlay = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3); 
	////cvShowImage("???",cvOverlay);
	// retrive properties
	RetriveProperties(cvOverlay);

	//classes
	ref_nb = (int *) malloc(7 * sizeof(int)); //hardcode 6 classes
	memset(ref_nb,0,7 * sizeof(int)); //default to 0

	sign_nb = (int *) malloc(7 * sizeof(int)); //hardcode 6 classes
	memset(sign_nb,0,7 * sizeof(int)); //default to 0

	if (!preserve)
	{
	//bucket for multiple image training

		BucketData.resize(100);
		BucketLabel1.resize(100);
		BucketLabel2.resize(100);
		BucketLabel3.resize(100);
		BucketLabel4.resize(100);
		BucketLabel5.resize(100);
		BucketLabel6.resize(100);
		BucketOverlay.resize(100);
		BucketCounts.resize(100);
		capsTrained = 0;


	}
	if (colourList.empty())
	{
		colourList.resize(7);
		for (int i = 0;i<7;i++)
		{
			colourList[i].resize(3);
		}

		
		colourList[0][2] = 0;//r
		colourList[0][1] = 0;//g
		colourList[0][0] = 0;//b
		//blue
		colourList[4][2] = 0;//r
		colourList[4][1] = 0;//g
		colourList[4][0] = 255;//b
		//yellow
		colourList[2][2] = 255;//r
		colourList[2][1] = 255;//g
		colourList[2][0] = 0;//b
		//cyan
		colourList[3][2] = 0;//r
		colourList[3][1] = 255;//g
		colourList[3][0] = 255;//b
		//green
		colourList[1][2] = 0;//r
		colourList[1][1] = 255;//g
		colourList[1][0] = 0;//b
		//magneta
		colourList[5][2] = 255;//r
		colourList[5][1] = 0;//g
		colourList[5][0] = 255;//b
		//white
		colourList[6][2] = 255;//r
		colourList[6][1] = 255;//g
		colourList[6][0] = 255;//b
	}
}

void TADDengine::loadClassifier(bool preserve){

	sPixels = 9200;
	segMethod = SEGMETHOD::XYZ_SLIC;
	segWeight = 0.1;
	mySeg = new FastFeatures(width,height,sPixels); 
	imgBuffer=(unsigned char*)malloc(width*height*sizeof(unsigned char)*4);
	memset(imgBuffer,0,width*height*sizeof(unsigned char)*4);
	imgBuffer4 = cvCreateImage( cvSize( width,height ), IPL_DEPTH_8U, 4);
	zeros = cvCreateMat(height,width, CV_8UC1);
	Adaboost = new Boosting(100);
	Background = new Backseg();
	threshBuffer=(unsigned char*)malloc(width*height*sizeof(unsigned char));
	cvOverlay = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3); 
	RetriveProperties(cvOverlay);
	ref_nb = (int *) malloc(7 * sizeof(int)); 
	memset(ref_nb,0,7 * sizeof(int));
	sign_nb = (int *) malloc(7 * sizeof(int)); 
	memset(sign_nb,0,7 * sizeof(int)); 

	/*if (!preserve)
	{
		BucketData.resize(100);
		BucketLabel1.resize(100);
		BucketLabel2.resize(100);
		BucketLabel3.resize(100);
		BucketLabel4.resize(100);
		BucketLabel5.resize(100);
		BucketLabel6.resize(100);
		BucketOverlay.resize(100);
		BucketCounts.resize(100);
		capsTrained = 0;
		
	}
	if (colourList.empty())
	{
		colourList.resize(7);
		for (int i = 0;i<7;i++)
		{
			colourList[i].resize(3);
		}
		colourList[0][2] = 0;//r
		colourList[0][1] = 0;//g
		colourList[0][0] = 0;//b
		colourList[1][2] = 0;//r
		colourList[1][1] = 255;//g
		colourList[1][0] = 0;//b
		colourList[2][2] = 255;//r
		colourList[2][1] = 0;//g
		colourList[2][0] = 0;//b
		colourList[3][2] = 0;//r
		colourList[3][1] = 0;//g
		colourList[3][0] = 255;//b
		colourList[4][2] = 255;//r
		colourList[4][1] = 255;//g
		colourList[4][0] = 0;//b
		colourList[5][2] = 178;//r
		colourList[5][1] = 0;//g
		colourList[5][0] = 255;//b
		colourList[6][2] = 255;//r
		colourList[6][1] = 108;//g
		colourList[6][0] = 40;//b
	}*/
}


int TADDengine::loadSettings(QString fname)
{
	///
	///Several default settings can be changed in the file Settings/settings.ini, 
	///this function loads them. If necessary it can also call FastFeatures::reAlloc() 
	///if changes require memory to be resized. fname always contains the full path 
	///of settings.ini

	if (fname.isEmpty())
	{
		return -1;
	}
	QFile settingsfile(fname);

	int thresh = -1;

	if (settingsfile.exists())
	{
		if (settingsfile.open(QIODevice::ReadOnly | QIODevice::Text))
		{
			char buf[1024];
			int err = settingsfile.readLine(buf,1023);
			
			while (err>0)
			{

				QByteArray ln(buf);

				QString strln(ln);
				if (err>6)
				{
					if (strln.left(6) == "Colour")
					{
						QStringList parsed = strln.split(",");
						int idx = parsed[1].toInt();
						colourList[idx][2] = parsed[2].toInt();
						colourList[idx][1] = parsed[3].toInt();
						colourList[idx][0] = parsed[4].toInt();

					}
					if (err>9)
					{
						if (strln.left(9) == "Threshold")
						{
							QStringList parsed = strln.split(",");
							thresh = parsed[1].toInt();

						}
						if (strln.left(10) == "Featureset")
						{
							QStringList parsed = strln.split(",");
							FEATSTEP = parsed[1].toInt()*7;
							mySeg->reAlloc();
						}
						if (strln.left(10) == "Segmenting")
						{
							QStringList parsed = strln.split(",");
							if (parsed[1].toAscii() == "SLIC")
							{
								segMethod=SEGMETHOD::SLIC;
							}
							else if(parsed[1].toAscii() == "XYZ_SLIC")
							{
								segMethod = SEGMETHOD::XYZ_SLIC;
							}
							else if(parsed[1].toAscii() == "RGB_SLIC")
							{
								segMethod = SEGMETHOD::RGB_SLIC;
							}
							segWeight = parsed[2].toFloat();

						}

					}
				}
				err = settingsfile.readLine(buf,1023);
			}
		}
	}

	return thresh;

}


IplImage* TADDengine::SLIC(IplImage* src, int draw){
	/**
	*This handles both segmentation and feature extraction. 
	*First calling FastImgSeg::LoadImg, then  FastImgSeg::DoSegmentation.
	*This complete it calls various feature extraction methods of 
	*FastFeatures, which inherits FastImgSeg.
	*/
	//converting 3 channel image to 4 channel image
	CvArr const * input[] = { src, zeros };
	CvArr * out[] = { imgBuffer4 };
	int from_to[] = { 0,0, 1,1, 2,2, 3,3 };
	CvArr * out2[] = {src, zeros };
	CvArr const * input2[] = { imgBuffer4 };


	// gSLIC currently only support 4-dimensional image 
	cvMixChannels(input, 2, out, 1, from_to, 4);
	imgBuffer    = (uchar*)imgBuffer4->imageData;

	//SLIC
	mySeg->LoadImg(imgBuffer);
	//mySeg->DoSegmentation(XYZ_SLIC,0.1, debugimg);
	//mySeg->DoSegmentation(SEGMETHOD::SLIC,0.3, 0);
	mySeg->DoSegmentation(segMethod,segWeight, 0);

	//cvSaveImage("Debugimage.png",dbgIplImg);
	
	//fetch label mask // used for overlay
	labelMask = mySeg->segMask;

	//execute the feature avg
	mySeg->Tool_NormalizedRGB();

	if (FEATSTEP>7)
	{
		mySeg->Tool_GetOtherFeats();
	}
	
	mySeg->Tool_GetAvgImg();
	mySeg->Tool_GetStdImg();
	mySeg->Tool_GetSkeImg();

	
	mySeg->Tool_GetEdgeImg();

	if(draw == 1)
	{
		mySeg->Tool_GetMarkedImg();
		// Draw Labels 
		imgBuffer = mySeg->markedImg;
		imgBuffer4->imageData = (char*)imgBuffer;

		cvMixChannels(input2, 1, out2, 2, from_to, 4);  //turn on to display superpixel overlay
	}

	return src;
}

IplImage* TADDengine::initOverlay(){
	/**This function will create an overlay image. In the current system this is 
	always called right before clearOverlay(), discarding anything 
	else it produces.*/
	//set all pixels of all channels to zero
	cvSet(cvOverlay , cvScalar(0,0,0));

	//set each superpixel as a query
	size = mySeg->Labels-mySeg->shrink;

	//create array to hold selected superpixels and the class
	overlayBuffer=(int*)malloc(size*sizeof(int));
	buffsize=size;
	memset(overlayBuffer,0,size*sizeof(int)); //anything greater than zero will be a labeled class
	backupIdx = 0;
	Matches.resize(size); // for background
		std::fill(Matches.begin(),Matches.end(),0);

	MatchesBG.resize(size); // for background
	std::fill(MatchesBG.begin(),MatchesBG.end(),0);

	return cvOverlay;
}

IplImage*  TADDengine::clearOverlay(){
	//Sets the contents of the overlay image to zeros.
	cvSet(cvOverlay , cvScalar(0,0,0));

	return cvOverlay;
}

IplImage* TADDengine::overlayFromBucket(int bucketIdx)
{
	///
	/// This code allows markup details to be stored in a format that can be accessed later.
	/// Currently unused.
	for (int i = 0;i<size;i++)
	{
		overlayBuffer[i] = BucketOverlay[bucketIdx][i];
	}
	for (int i = 0;i<6;i++)
	{
		ref_nb[i] = BucketCounts[bucketIdx][i];
	}
	generateOverlay();
	return cvOverlay;
}

void TADDengine::generateOverlay() {
	/**The main Overlay function, this gets called by updateOverlay, 
	overlayFromBucket, Undo and Redo, to create the actual overlay 
	image to display.*/
	int widthx = width;

	tbb::parallel_for (int (0), height, [&, widthx](int h) {
		for (int w = 0; w < widthx; w++) {
			int labelIndex = h*widthx+w;
			int labelValue = labelMask[labelIndex];
			if(MatchesBG[labelValue] == 0){
				
				cvData[h*step + w*nChannels + 0] = colourList[overlayBuffer[labelValue]][0];
				cvData[h*step + w*nChannels + 1] = colourList[overlayBuffer[labelValue]][1];
				cvData[h*step + w*nChannels + 2] = colourList[overlayBuffer[labelValue]][2];
				
			}
		}
	});
	

	
}



void TADDengine::generateOverlayClassBoosting() {
	/**
	Updates the overlay using the outputs from classification, to display the areas classified.
	*/
		tbb::parallel_for (int (0), height, [&](int h) {
		for (int w = 0; w < width; w++) {
			int labelIndex = h*width+w;
			int labelValue = labelMask[labelIndex];
			int colClass;
			if(MatchesBG[labelValue] == 0)
			{
				colClass = signV[labelValue];
			}
			else
			{
				colClass = 0;
			}

			cvData[h*step + w*nChannels + 0] = colourList[colClass][0];
			cvData[h*step + w*nChannels + 1] = colourList[colClass][1];
			cvData[h*step + w*nChannels + 2] = colourList[colClass][2];


		}
	});
}

IplImage* TADDengine::updateOverlay(QPoint clickPos, int classLabel) {
	/**Processes changes to the overlay from clicks on the markup window. 
	Calls generateOverlay to create the finished image.*/
	mskIndex=clickPos.y()*width+clickPos.x();
	if(clickPos.y() < height && clickPos.x() < width && clickPos.y() > 0 && clickPos.x() > 0)
	{
		selectedLabel = labelMask[mskIndex];

		//label is being reclassifed
		if (classLabel > 0 && overlayBuffer[selectedLabel] != 0) 
		{
			//decrease previous class
			ref_nb[overlayBuffer[selectedLabel]]--; 
			//increase new class
			ref_nb[classLabel]++; 
		}
		else//if (classLabel > 0 && overlayBuffer[selectedLabel] == 0) 
		{
			//increase ref_nb count of class label
			ref_nb[classLabel]++; 
		}

		if (classLabel == 0 && overlayBuffer[selectedLabel] != 0) 
		{
			//decrease ref_nb count of class label
			ref_nb[overlayBuffer[selectedLabel]]--; 
		}

		overlayBuffer[selectedLabel] = classLabel; 
		//generate overlay image
		generateOverlay();
	}

	//return overlay
	return cvOverlay;
}

//IplImage* TADDengine::classifyOverlayKNN(bool isClassify){
//
//	cvOverlay = initOverlay();
//
//	k = 15;
//	query_nb   = size;   // Query point number,     max=65535
//	ref_classes = 4;   //number of classes 
//	dim = 3;		// Dimension of points
//	
//	query  = (float *) malloc(query_nb * dim * sizeof(float));
//	ind    = (int *)   malloc(query_nb * k * sizeof(float));
//	dist_all  = (float **) malloc(ref_classes * sizeof(float*));
//
//	//allocate distance arrays
//	for(int cl = 0; cl < ref_classes; cl++)
//	{
//		dist_all[cl] = (float *) malloc(query_nb * k * sizeof(float));
//	}
//	
//	
//	//allocate querys
//		//Concurrency::parallel_for (int (0), size, [&](int s) {
//		for(int s = 0; s < size; s++)
//		{
//			query[s] = mySeg->avgStore[s*4+1];
//			query[s+(size)] = mySeg->avgStore[s*4+2];
//			query[s+(size*2)] = mySeg->avgStore[s*4+3];
//		}
//		//});
//
//	if(isClassify)
//	{
//	 labelClass = myClass->Classify(ref,	30,	ref_classes,	query,	query_nb,	dim,	k,	dist_all,	ind);
//
//	  //generate overlay
//	   generateOverlayClass();
//	}
//
//	free(query);
//	free(ind);
//
//	for(int i = 0; i < ref_classes; i++)
//	{
//		free(dist_all[i]);
//	}
//
//	free(dist_all);
//	free(overlayBuffer);
//
//	//return overlay
//	return cvOverlay;
//}

void TADDengine::trainBackground(IplImage* src){
	/**Unused, routines for removing the background which have been 
	superceded by the use of a deliberately dark background.
	*/
	//generate Data
	std::vector<std::vector<float>> inData(3, std::vector<float>(width*height));

	/*
	for(int k = 0; k <size; k++){
			float r = mySeg->avgStore[k*8+1]; //r
			float g = mySeg->avgStore[k*8+2]; //g
			float b = mySeg->avgStore[k*8+3]; //b
			float rn = mySeg->avgStore[k*8+4]; //rn
			float gn = mySeg->avgStore[k*8+5]; //gb
			float bn = mySeg->avgStore[k*8+6]; //bn
			float rskw = mySeg->skw1Store[k*7+0];
			float gskw = mySeg->skw1Store[k*7+1];
			float bskw = mySeg->skw1Store[k*7+2];

			inData[0][k] = r; inData[6][k] = rskw; 
			inData[1][k] = g; inData[7][k] = gskw; 
			inData[2][k] = b; inData[8][k] = bskw; 
			inData[3][k] = rn; 
			inData[4][k] = gn; 
			inData[5][k] = bn; 
	}
	*/

	unsigned char* srcBuffer    = (uchar*)src->imageData;
		tbb::parallel_for (int (0), height, [&](int h) {
		for (int w = 0; w < width; w++) {
			int Index = h*width+w;

			inData[0][Index] = (float)srcBuffer[Index*3+2];
			inData[1][Index] = (float)srcBuffer[Index*3+1];
			inData[2][Index] = (float)srcBuffer[Index*3+0];
		}
		});

	Background->LearnBackground(inData);
}

IplImage* TADDengine::UpdateBackground(QPoint clickPos, IplImage* src, int Value) {
	IplImage* BacksegImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3); 
	cvCopy(src,BacksegImage);
	/**Unused, routines for removing the background which have been 
	superceded by the use of a deliberately dark background.
	*/
	
	mskIndex=clickPos.y()*width+clickPos.x();
	

	if(clickPos.y() < height && clickPos.x() < width && clickPos.y() > 0 && clickPos.x() > 0)
	{
		selectedLabel = labelMask[mskIndex];

		//generate Data
		std::vector<std::vector<float>> inData(9, std::vector<float>(1));

		float r = mySeg->avgStore[selectedLabel*8+1]; //r
		float g = mySeg->avgStore[selectedLabel*8+2]; //g
		float b = mySeg->avgStore[selectedLabel*8+3]; //b
		float rn = mySeg->avgStore[selectedLabel*8+4]; //rn
		float gn = mySeg->avgStore[selectedLabel*8+5]; //gb
		float bn = mySeg->avgStore[selectedLabel*8+6]; //bn
		float rskw = mySeg->skw1Store[selectedLabel*7+0];
		float gskw = mySeg->skw1Store[selectedLabel*7+1];
		float bskw = mySeg->skw1Store[selectedLabel*7+2];

		inData[0][0] = r; inData[6][0] = rskw; 
		inData[1][0] = g; inData[7][0] = gskw; 
		inData[2][0] = b; inData[8][0] = bskw; 
		inData[3][0] = rn; 
		inData[4][0] = gn; 
		inData[5][0] = bn; 


		Background->UpdateBackground(inData,Value);
	}

	//remove background
	BacksegImage = removeBackground(src);

	return BacksegImage;
}

IplImage* TADDengine::removeBackground(IplImage* src){
	/**Unused, routines for removing the background which have been 
	superceded by the use of a deliberately dark background.
	*/
	
	//generate Data
	std::vector<std::vector<float>> inData(3, std::vector<float>(width*height));

	/*
		for(int k = 0; k <size; k++){
			float r = mySeg->avgStore[k*8+1]; //r
			float g = mySeg->avgStore[k*8+2]; //g
			float b = mySeg->avgStore[k*8+3]; //b
			float rn = mySeg->avgStore[k*8+4]; //rn
			float gn = mySeg->avgStore[k*8+5]; //gb
			float bn = mySeg->avgStore[k*8+6]; //bn
			float rskw = mySeg->skw1Store[k*7+0];
			float gskw = mySeg->skw1Store[k*7+1];
			float bskw = mySeg->skw1Store[k*7+2];

			inData[0][k] = r; inData[6][k] = rskw; 
			inData[1][k] = g; inData[7][k] = gskw; 
			inData[2][k] = b; inData[8][k] = bskw; 
			inData[3][k] = rn; 
			inData[4][k] = gn; 
			inData[5][k] = bn; 
	}
	*/
	unsigned char* srcBuffer    = (uchar*)src->imageData;
		tbb::parallel_for (int (0), height, [&](int h) {
		for (int w = 0; w < width; w++) {
			int Index = h*width+w;

			inData[0][Index] = srcBuffer[Index*3+2];
			inData[1][Index] = srcBuffer[Index*3+1];
			inData[2][Index] = srcBuffer[Index*3+0];
		}
		});



	//match
	Matches = Background->RemoveBackground(inData,1);

	//create blank image
	IplImage* BacksegImage = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3); 

	cvCopy(src,BacksegImage,0);
	cvShowImage("Example1", BacksegImage);
	/* retrieve properties */
	//Frame
		int nchannelsF =  BacksegImage->nChannels;
		int stepF      =  BacksegImage->widthStep;
		uchar* dataF	   = (uchar*)BacksegImage->imageData; 

	//remove postive superpixels
	tbb::parallel_for (int (0), height, [&](int h) {
		for (int w = 0; w < width; w++) {
			int Index = h*width+w;

			if(Matches[Index] == 1){
				dataF[h*stepF + w*nchannelsF + 0] = 0; //R
				dataF[h*stepF + w*nchannelsF + 1] = 0; //G
				dataF[h*stepF + w*nchannelsF + 2] = 0; //B
			}
		}
	});


	return BacksegImage;
}

void TADDengine::KeepBackup()
{
	/**Keeps a record of the current image markup for the undo/redo functions
	*/
	int*backup;
	if (backupIdx==0)
		backupIdx=1;
	backup = (int*)malloc(buffsize*sizeof(int));
	int * backup_ref_nb;
	backup_ref_nb = (int*)malloc(6*sizeof(int));
	
	memcpy(backup,overlayBuffer,buffsize*sizeof(int));
	memcpy(backup_ref_nb,ref_nb,6*sizeof(int));
	if (unredo.size() > backupIdx)
	{
		//Remove everything from backupIdx to the end
		while(unredo.size()>backupIdx)
		{
			free(unredo.last());
			unredo.removeLast();
			free(unredo_ref_nb.last());
			unredo_ref_nb.removeLast();
		}
	}
	unredo.append(backup);
	unredo_ref_nb.append(backup_ref_nb);
	backupIdx = unredo.size();
	//msgBox.setText(QString::fromAscii("backup Index=") + QString::number(backupIdx));
//	msgBox.exec();
}

IplImage * TADDengine::Undo()
{
	/* Backup the current markup and revert to a previous backup if one exists.
	*/
	if (backupIdx==0)
	{
		generateOverlay();
		return cvOverlay;
	}
	if (backupIdx==unredo.size())
	{
		KeepBackup();
		backupIdx--;
	}

	memcpy(overlayBuffer,unredo.at(--backupIdx),buffsize*sizeof(int));
	memcpy(ref_nb,unredo_ref_nb.at(backupIdx),6*sizeof(int));

	generateOverlay();
	return cvOverlay;
}

IplImage * TADDengine::Redo()
{
	/** revert to a more recent backup if we are currently using a backed up
	 version of the user markup.*/

	if (backupIdx>=unredo.size()-1)
	{
		generateOverlay();
		return cvOverlay;
	}

	memcpy(overlayBuffer,unredo.at(++backupIdx),buffsize*sizeof(int));
	memcpy(ref_nb,unredo_ref_nb.at(backupIdx),6*sizeof(int));

	generateOverlay();
	return cvOverlay;
}

void TADDengine::updateMatches(IplImage* src, IplImage* threshold){

	int labelsSize = MatchesBG.size();

	float* treshStore=(float*) malloc(1*labelsSize*sizeof(float));   
	memset(treshStore,0,1*labelsSize*sizeof(float));
	
	threshBuffer    = (uchar*)threshold->imageData;
		//for (int i=0;i<height;i++) {
		tbb::parallel_for (int (0), height, [&](int i) {
			for (int j=0;j<width;j++)
			{
				int mskIndex=i*width+j;
				int msk = labelMask[mskIndex];

	            if (threshBuffer[mskIndex] == 1)
				{
					treshStore[msk]++;
				}

			}
		});

		for(int i=0; i < labelsSize; i++){

			float amount = mySeg->avgStore[i*(FEATSTEP+1)+0]; //r
			float count = treshStore[i];

			//if less than 85% of the pixels are inside the potato discard the super pixel
				if( count != amount ){
				    MatchesBG[i] = 1;
			   }

		}
/*
	   for (int i=0;i<height;i++) {
			for (int j=0;j<width;j++)
			{
				int mskIndex=i*width+j;
				int msk = labelMask[mskIndex];

				float amount = mySeg->avgStore[msk*8+0]; //r
				float count = treshStore[msk];

				//if less than 29 pixes are in thresholded on in the superpixel its background
				if( count < 70 ){
				    MatchesBG[msk] = 1;
			   }
			}
		}
		*/

		/*
	    tbb::parallel_for (int (0), height, [&](int h) {
		for (int w = 0; w < width; w++) {
			int labelIndex = h*width+w;
			int labelValue = labelMask[labelIndex];

			float r = mySeg->avgStore[labelValue*8+1]; //r
			float g = mySeg->avgStore[labelValue*8+2]; //g
			float b = mySeg->avgStore[labelValue*8+3]; //b

			if( r <= 10 && g <= 10  && b <= 10 ){

				MatchesBG[labelValue] = 1;
			}
		}
		});
		*/
		int  t = MatchesBG[1];

		free(treshStore);
}

IplImage* TADDengine::classifyOverlayAdaboost(bool isClassify){
	/**The main classification function, this classifies every non-background 
	superpixel in the image and calls generateOverlayClassBoosting to produce 
	the actual image.*/

	//cvOverlay = initOverlay();

	//create data vector with all features for each label
		std::vector<std::vector<float>> Data((3*FEATSTEP), std::vector<float>(size)); //Data vector =+ avg6 + var6 + ske6 = 18

		std::vector<float> counts(size);

		if(isClassify) {

			
		tbb::parallel_for(int(0),size,[&](int k)
		//for(int k = 0; k <size; k++)
		{
			int featType=0;

			counts[k] = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7) + 0]; // number of pixels in this superpixel

			float r = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+1]; //r
			float g = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+2]; //g
			float b = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+3]; //b
			float rn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+4]; //rn
			float gn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+5]; //gb
			float bn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+6]; //bn
			float i = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+7]; //i
			float rvar = mySeg->varStore[k*FEATSTEP+(featType*7)+0];
			float gvar = mySeg->varStore[k*FEATSTEP+(featType*7)+1];
			float bvar= mySeg->varStore[k*FEATSTEP+(featType*7)+2];
			float rnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+3];
			float gnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+4];
			float bnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+5];
			float ivar = mySeg->varStore[k*FEATSTEP+(featType*7)+6]; //i
			float rskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+0];
			float gskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+1];
			float bskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+2];
			float rnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+3];
			float gnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+4];
			float bnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+5];
			float iskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+6]; //i
			
			Data[0][k] = r; Data[6][k] = rvar; Data[12][k] = rskw; 
			Data[1][k] = g; Data[7][k] = gvar; Data[13][k] = gskw;
			Data[2][k] = b; Data[8][k] = bvar; Data[14][k] = bskw;
			Data[3][k] = rn; Data[9][k] = rnvar; Data[15][k] = rnskw;
			Data[4][k] = gn; Data[10][k] = gnvar; Data[16][k] = gnskw;
			Data[5][k] = bn; Data[11][k] = bnvar; Data[17][k] = bnskw;

			Data[18][k] = i;
			Data[19][k] = ivar;
			Data[20][k] = iskw;
			if (FEATSTEP>7)
			{
				featType=1;
				float r = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+1]; //r
				float g = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+2]; //g
				float b = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+3]; //b
				float rn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+4]; //rn
				float gn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+5]; //gb
				float bn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+6]; //bn
				float i = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+7]; //i
				float rvar = mySeg->varStore[k*FEATSTEP+(featType*7)+0];
				float gvar = mySeg->varStore[k*FEATSTEP+(featType*7)+1];
				float bvar= mySeg->varStore[k*FEATSTEP+(featType*7)+2];
				float rnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+3];
				float gnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+4];
				float bnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+5];
				float ivar = mySeg->varStore[k*FEATSTEP+(featType*7)+6]; //i
				float rskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+0];
				float gskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+1];
				float bskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+2];
				float rnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+3];
				float gnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+4];
				float bnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+5];
				float iskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+6]; //i

				Data[(21*featType)+0][k] = r; Data[(21*featType)+6][k] = rvar; Data[(21*featType)+12][k] = rskw; 
				Data[(21*featType)+1][k] = g; Data[(21*featType)+7][k] = gvar; Data[(21*featType)+13][k] = gskw;
				Data[(21*featType)+2][k] = b; Data[(21*featType)+8][k] = bvar; Data[(21*featType)+14][k] = bskw;
				Data[(21*featType)+3][k] = rn; Data[(21*featType)+9][k] = rnvar; Data[(21*featType)+15][k] = rnskw;
				Data[(21*featType)+4][k] = gn; Data[(21*featType)+10][k] = gnvar; Data[(21*featType)+16][k] = gnskw;
				Data[(21*featType)+5][k] = bn; Data[(21*featType)+11][k] = bnvar; Data[(21*featType)+17][k] = bnskw;
				
				Data[(21*featType)+18][k] = i;
				Data[(21*featType)+19][k] = ivar;
				Data[(21*featType)+20][k] = iskw;
			}
			if (FEATSTEP>14)
			{
				featType=2;
				float r = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+1]; //r
				float g = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+2]; //g
				float b = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+3]; //b
				float rn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+4]; //rn
				float gn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+5]; //gb
				float bn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+6]; //bn
				float i = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+7]; //i
				float rvar = mySeg->varStore[k*FEATSTEP+(featType*7)+0];
				float gvar = mySeg->varStore[k*FEATSTEP+(featType*7)+1];
				float bvar= mySeg->varStore[k*FEATSTEP+(featType*7)+2];
				float rnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+3];
				float gnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+4];
				float bnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+5];
				float ivar = mySeg->varStore[k*FEATSTEP+(featType*7)+6]; //i
				float rskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+0];
				float gskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+1];
				float bskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+2];
				float rnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+3];
				float gnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+4];
				float bnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+5];
				float iskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+6]; //i

				Data[(21*featType)+0][k] = r; Data[(21*featType)+6][k] = rvar; Data[(21*featType)+12][k] = rskw; 
				Data[(21*featType)+1][k] = g; Data[(21*featType)+7][k] = gvar; Data[(21*featType)+13][k] = gskw;
				Data[(21*featType)+2][k] = b; Data[(21*featType)+8][k] = bvar; Data[(21*featType)+14][k] = bskw;
				Data[(21*featType)+3][k] = rn; Data[(21*featType)+9][k] = rnvar; Data[(21*featType)+15][k] = rnskw;
				Data[(21*featType)+4][k] = gn; Data[(21*featType)+10][k] = gnvar; Data[(21*featType)+16][k] = gnskw;
				Data[(21*featType)+5][k] = bn; Data[(21*featType)+11][k] = bnvar; Data[(21*featType)+17][k] = bnskw;
				
				Data[(21*featType)+18][k] = i;
				Data[(21*featType)+19][k] = ivar;
				Data[(21*featType)+20][k] = iskw;
			}
			if (FEATSTEP>21)
			{
				featType=3;
				float r = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+1]; //r
				float g = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+2]; //g
				float b = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+3]; //b
				float rn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+4]; //rn
				float gn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+5]; //gb
				float bn = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+6]; //bn
				float i = mySeg->avgStore[k*(FEATSTEP+1)+(featType*7)+7]; //i
				float rvar = mySeg->varStore[k*FEATSTEP+(featType*7)+0];
				float gvar = mySeg->varStore[k*FEATSTEP+(featType*7)+1];
				float bvar= mySeg->varStore[k*FEATSTEP+(featType*7)+2];
				float rnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+3];
				float gnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+4];
				float bnvar = mySeg->varStore[k*FEATSTEP+(featType*7)+5];
				float ivar = mySeg->varStore[k*FEATSTEP+(featType*7)+6]; //i
				float rskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+0];
				float gskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+1];
				float bskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+2];
				float rnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+3];
				float gnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+4];
				float bnskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+5];
				float iskw = mySeg->skw1Store[k*FEATSTEP+(featType*7)+6]; //i

				Data[(21*featType)+0][k] = r; Data[(21*featType)+6][k] = rvar; Data[(21*featType)+12][k] = rskw; 
				Data[(21*featType)+1][k] = g; Data[(21*featType)+7][k] = gvar; Data[(21*featType)+13][k] = gskw;
				Data[(21*featType)+2][k] = b; Data[(21*featType)+8][k] = bvar; Data[(21*featType)+14][k] = bskw;
				Data[(21*featType)+3][k] = rn; Data[(21*featType)+9][k] = rnvar; Data[(21*featType)+15][k] = rnskw;
				Data[(21*featType)+4][k] = gn; Data[(21*featType)+10][k] = gnvar; Data[(21*featType)+16][k] = gnskw;
				Data[(21*featType)+5][k] = bn; Data[(21*featType)+11][k] = bnvar; Data[(21*featType)+17][k] = bnskw;
				
				Data[(21*featType)+18][k] = i;
				Data[(21*featType)+19][k] = ivar;
				Data[(21*featType)+20][k] = iskw;
			}
		});
	//classify

		vector<float> classifed1;
		vector<float> classifed2;
		vector<float> classifed3;
		vector<float> classifed4;
		vector<float> classifed5;
		vector<float> classifed6;

		
		tbb::parallel_invoke(
		[&]{classifed1 =  Adaboost->Classify(get<0>(Classifier1), get<1>(Classifier1), Data, (int)Data.size(), size);},
		[&]{classifed2 =  Adaboost->Classify(get<0>(Classifier2), get<1>(Classifier2), Data, (int)Data.size(), size);},
		[&]{classifed3 =  Adaboost->Classify(get<0>(Classifier3), get<1>(Classifier3), Data, (int)Data.size(), size);},
		[&]{classifed4 =  Adaboost->Classify(get<0>(Classifier4), get<1>(Classifier4), Data, (int)Data.size(), size);},
		[&]{classifed5 =  Adaboost->Classify(get<0>(Classifier5), get<1>(Classifier5), Data, (int)Data.size(), size);},
		[&]{classifed6 =  Adaboost->Classify(get<0>(Classifier6), get<1>(Classifier6), Data, (int)Data.size(), size);}
		);

		//vector<float> classifed1 =  Adaboost->Classify(get<0>(Classifier1), get<1>(Classifier1), Data, (int)Data.size(), size);
		//vector<float> classifed2 =  Adaboost->Classify(get<0>(Classifier2), get<1>(Classifier2), Data, (int)Data.size(), size);
		//vector<float> classifed3 =  Adaboost->Classify(get<0>(Classifier3), get<1>(Classifier3), Data, (int)Data.size(), size);
		//vector<float> classifed4 =  Adaboost->Classify(get<0>(Classifier4), get<1>(Classifier4), Data, (int)Data.size(), size);

		if (!repClass[0])
		{
			tbb::parallel_for(int(0),size,[&](int k) {
				classifed1[k] = -FLT_MAX;
			});
		}
		if (!repClass[1])
		{
			tbb::parallel_for(int(0),size,[&](int k) {
				classifed2[k] = -FLT_MAX;
			});
		}
		if (!repClass[2])
		{
			tbb::parallel_for(int(0),size,[&](int k) {
				classifed3[k] = -FLT_MAX;
			});
		}
		if (!repClass[3])
		{
			tbb::parallel_for(int(0),size,[&](int k) {
				classifed4[k] = -FLT_MAX;
			});
		}
		if (!repClass[4])
		{
			tbb::parallel_for(int(0),size,[&](int k) {
				classifed5[k] = -FLT_MAX;
			});
		}
		if (!repClass[5])
		{
			tbb::parallel_for(int(0),size,[&](int k) {
				classifed6[k] = -FLT_MAX;
			});
		}

     //sign
      memset(sign_nb,0,7 * sizeof(int)); //default to 0
		//signV = Adaboost->sign(classifed1, (int)Data.size(), size);
		 signV = Adaboost->sign4(classifed1, classifed2, classifed3, classifed4, classifed5, classifed6, (int)Data.size(), size, sign_nb, MatchesBG, counts);

	//generate overlay
		generateOverlayClassBoosting();
		}
        
	//free(overlayBuffer);

	return cvOverlay;

}


void TADDengine::refKNN(){
	/**Unused, KNN is no longer a part of this system. Kept for historical reasons.*/
	//int    ref_nb     = 10;   // Reference point number, max=65535
	ref_classes = 4;   //number of classes 
	dim = 3;		// Dimension of points
	ref    = (float **) malloc(ref_classes * sizeof(float*));

	//allocate arrays
	for(int cl = 0; cl < ref_classes; cl++)
	{
		ref[cl] = (float *) malloc(ref_nb[cl+1] * dim * sizeof(float));
	}
	
	int f = 0;
	//populate ref array
	for(int cl =0; cl <  ref_classes; cl++)
	{
		for(int i =0; i < ref_nb[cl+1]; i++)
		{
			//find next label
			while(f < size)
			{
				if( overlayBuffer[f] == (cl+1))
					break;
				f++;
			}

			//first reb_nv[cl+1] = red values, then green, blue,
			ref[cl][i+(0*ref_nb[cl+1])] = mySeg->avgStore[f*4+1];
			ref[cl][i+(1*ref_nb[cl+1])] = mySeg->avgStore[f*4+2];
			ref[cl][i+(2*ref_nb[cl+1])] = mySeg->avgStore[f*4+3];
		}
		//reset f for next class
		f = 0;
	}
	
}

void TADDengine::RetriveProperties(IplImage *cvOverlay) {
	/*Updates class variables such as height and width with 
	details of the images being processed.*/
	nChannels	=	cvOverlay->nChannels;
	step		=	cvOverlay->widthStep;
	depth		=   cvOverlay->depth;
	cvData	    =	(uchar*)cvOverlay->imageData; 
}

void TADDengine::refAdaboost(int idx){
	/** 
	*	Adds the currently marked up superpixels to the training data, or overwrites a previous 
	*	subset of the training data if idx is the index of a previously trained image, then calls 
	*	Boosting::RealAdaboost() to create the classifiers.
	*	This function now takes a number, idx, which represents the image being captured from. If
	*	training data has already been captured from the same image, that training data will be
	*	overwritten to avoid training from the same image multiple times.
	*
	*	This feature is implemented to allow re-training from previous images.
	*/
		//overlay buffer contains labels selected with class identifer

		//ref_nb 0-1 contains the amount of each class
	    int labelsSize = ref_nb[1] + ref_nb[2] + ref_nb[3] + ref_nb[4] + ref_nb[5] + ref_nb[6];
		//vector Labels = size (ref_nb[0] + ref_nb[1])
		std::vector<int> Labels1(labelsSize);
		std::vector<int> DataMask1(labelsSize);
		

		std::vector<int> Labels2(labelsSize);
		std::vector<int> DataMask2(labelsSize);
		

		std::vector<int> Labels3(labelsSize);
		std::vector<int> DataMask3(labelsSize);
		

		std::vector<int> Labels4(labelsSize);
		std::vector<int> DataMask4(labelsSize);

		std::vector<int> Labels5(labelsSize);
		std::vector<int> DataMask5(labelsSize);

		std::vector<int> Labels6(labelsSize);
		std::vector<int> DataMask6(labelsSize);

		std::vector<std::vector<float>> Data((3*FEATSTEP), std::vector<float>(labelsSize)); //Data vector =+ avg6 + var6 + ske6 = 18


		// Memory hack, delete them and reassign them to prevent them being reallocated over and over.
		static tree_node_w *weak_learner1 = new tree_node_w(6,100);
		static tree_node_w *weak_learner2 = new tree_node_w(6,100);
		static tree_node_w *weak_learner3 = new tree_node_w(6,100);
		static tree_node_w *weak_learner4 = new tree_node_w(6,100);
		static tree_node_w *weak_learner5 = new tree_node_w(6,100);
		static tree_node_w *weak_learner6 = new tree_node_w(6,100);

		delete(weak_learner1);
		delete(weak_learner2);
		delete(weak_learner3);
		delete(weak_learner4);
		delete(weak_learner5);
		delete(weak_learner6);

		weak_learner1 = new tree_node_w(6,100);
		weak_learner2 = new tree_node_w(6,100);
		weak_learner3 = new tree_node_w(6,100);
		weak_learner4 = new tree_node_w(6,100);
		weak_learner5 = new tree_node_w(6,100);
		weak_learner6 = new tree_node_w(6,100);


		for (int i = 0;i<6;i++)
		{
			repClass[i]=false;
		}

		//class one = postive, class two = negative -- i = label
		int k = 0;
		for(int i = 0; i < size; i++){
			if(overlayBuffer[i] == 1){
				Labels1[k] = 1;
				DataMask1[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 2){
				Labels1[k] = -1;
				DataMask1[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 3){
				Labels1[k] = -1;
				DataMask1[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 4){
				Labels1[k] = -1;
				DataMask1[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 5){
				Labels1[k] = -1;
				DataMask1[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 6){
				Labels1[k] = -1;
				DataMask1[k] = i;
				k++;
			}
		}

		k = 0;
		for(int i = 0; i < size; i++){
			if(overlayBuffer[i] == 1){
				Labels2[k] = -1;
				DataMask2[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 2){
				Labels2[k] = 1;
				DataMask2[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 3){
				Labels2[k] = -1;
				DataMask2[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 4){
				Labels2[k] = -1;
				DataMask2[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 5){
				Labels2[k] = -1;
				DataMask2[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 6){
				Labels2[k] = -1;
				DataMask2[k] = i;
				k++;
			}
		}

		k = 0;
		for(int i = 0; i < size; i++){
			if(overlayBuffer[i] == 1){
				Labels3[k] = -1;
				DataMask3[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 2){
				Labels3[k] = -1;
				DataMask3[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 3){
				Labels3[k] = 1;
				DataMask3[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 4){
				Labels3[k] = -1;
				DataMask3[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 5){
				Labels3[k] = -1;
				DataMask3[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 6){
				Labels3[k] = -1;
				DataMask3[k] = i;
				k++;
			}
		}

		k = 0;
		for(int i = 0; i < size; i++){
			if(overlayBuffer[i] == 1){
				Labels4[k] = -1;
				DataMask4[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 2){
				Labels4[k] = -1;
				DataMask4[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 3){
				Labels4[k] = -1;
				DataMask4[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 4){
				Labels4[k] = 1;
				DataMask4[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 5){
				Labels4[k] = -1;
				DataMask4[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 6){
				Labels4[k] = -1;
				DataMask4[k] = i;
				k++;
			}
		}

		k = 0;
		for(int i = 0; i < size; i++){
			if(overlayBuffer[i] == 1){
				Labels5[k] = -1;
				DataMask5[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 2){
				Labels5[k] = -1;
				DataMask5[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 3){
				Labels5[k] = -1;
				DataMask5[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 4){
				Labels5[k] = -1;
				DataMask4[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 5){
				Labels5[k] = 1;
				DataMask5[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 6){
				Labels5[k] = -1;
				DataMask5[k] = i;
				k++;
			}
		}

		k = 0;
		for(int i = 0; i < size; i++){
			if(overlayBuffer[i] == 1){
				Labels6[k] = -1;
				DataMask6[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 2){
				Labels6[k] = -1;
				DataMask6[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 3){
				Labels6[k] = -1;
				DataMask6[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 4){
				Labels6[k] = -1;
				DataMask6[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 5){
				Labels6[k] = -1;
				DataMask6[k] = i;
				k++;
			}
			if(overlayBuffer[i] == 6){
				Labels6[k] = 1;
				DataMask6[k] = i;
				k++;
			}
		}
	    //DataLine = size (ref_nb[0] + ref_nb[1]);
		//for(int k = 0; k < (int)DataMask1.size(); k++){
		tbb::parallel_for(int(0),(int)DataMask1.size(),[&](int k) {
			int featType = 0;
			int tl = DataMask1[k];

			float count = mySeg->avgStore[tl*(FEATSTEP+1)+0];

			float r = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+1]; //r
			float g = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+2]; //g
			float b = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+3]; //b
			float rn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+4]; //rn
			float gn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+5]; //gb
			float bn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+6]; //bn
			float i = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+7]; //i
			float rvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+0];
			float gvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+1];
			float bvar= mySeg->varStore[tl*FEATSTEP+(featType*7)+2];
			float rnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+3];
			float gnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+4];
			float bnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+5];
			float ivar = mySeg->varStore[tl*FEATSTEP+(featType*7)+6]; //i
			float rskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+0];
			float gskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+1];
			float bskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+2];
			float rnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+3];
			float gnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+4];
			float bnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+5];
			float iskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+6]; //i
			
			Data[0][k] = r; Data[(21*featType)+6][k] = rvar; Data[(21*featType)+12][k] = rskw; 
			Data[(21*featType)+1][k] = g; Data[(21*featType)+7][k] = gvar; Data[(21*featType)+13][k] = gskw;
			Data[(21*featType)+2][k] = b; Data[(21*featType)+8][k] = bvar; Data[(21*featType)+14][k] = bskw;
			Data[(21*featType)+3][k] = rn; Data[(21*featType)+9][k] = rnvar; Data[(21*featType)+15][k] = rnskw;
			Data[(21*featType)+4][k] = gn; Data[(21*featType)+10][k] = gnvar; Data[(21*featType)+16][k] = gnskw;
			Data[(21*featType)+5][k] = bn; Data[(21*featType)+11][k] = bnvar; Data[(21*featType)+17][k] = bnskw;

			Data[(21*featType)+18][k] = i;
			Data[(21*featType)+19][k] = ivar;
			Data[(21*featType)+20][k] = iskw;

			if (FEATSTEP>7)
			{
				int featType = 1;
				int tl = DataMask1[k];

				float count = mySeg->avgStore[tl*(FEATSTEP+1)+0];

				float r = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+1]; //r
				float g = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+2]; //g
				float b = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+3]; //b
				float rn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+4]; //rn
				float gn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+5]; //gb
				float bn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+6]; //bn
				float i = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+7]; //i
				float rvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+0];
				float gvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+1];
				float bvar= mySeg->varStore[tl*FEATSTEP+(featType*7)+2];
				float rnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+3];
				float gnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+4];
				float bnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+5];
				float ivar = mySeg->varStore[tl*FEATSTEP+(featType*7)+6]; //i
				float rskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+0];
				float gskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+1];
				float bskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+2];
				float rnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+3];
				float gnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+4];
				float bnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+5];
				float iskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+6]; //i

				Data[0][k] = r; Data[(21*featType)+6][k] = rvar; Data[(21*featType)+12][k] = rskw; 
				Data[(21*featType)+1][k] = g; Data[(21*featType)+7][k] = gvar; Data[(21*featType)+13][k] = gskw;
				Data[(21*featType)+2][k] = b; Data[(21*featType)+8][k] = bvar; Data[(21*featType)+14][k] = bskw;
				Data[(21*featType)+3][k] = rn; Data[(21*featType)+9][k] = rnvar; Data[(21*featType)+15][k] = rnskw;
				Data[(21*featType)+4][k] = gn; Data[(21*featType)+10][k] = gnvar; Data[(21*featType)+16][k] = gnskw;
				Data[(21*featType)+5][k] = bn; Data[(21*featType)+11][k] = bnvar; Data[(21*featType)+17][k] = bnskw;

				Data[(21*featType)+18][k] = i;
				Data[(21*featType)+19][k] = ivar;
				Data[(21*featType)+20][k] = iskw;
			}			
			if (FEATSTEP>14)
			{
				int featType = 2;
				int tl = DataMask1[k];

				float count = mySeg->avgStore[tl*(FEATSTEP+1)+0];

				float r = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+1]; //r
				float g = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+2]; //g
				float b = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+3]; //b
				float rn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+4]; //rn
				float gn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+5]; //gb
				float bn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+6]; //bn
				float i = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+7]; //i
				float rvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+0];
				float gvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+1];
				float bvar= mySeg->varStore[tl*FEATSTEP+(featType*7)+2];
				float rnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+3];
				float gnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+4];
				float bnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+5];
				float ivar = mySeg->varStore[tl*FEATSTEP+(featType*7)+6]; //i
				float rskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+0];
				float gskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+1];
				float bskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+2];
				float rnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+3];
				float gnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+4];
				float bnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+5];
				float iskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+6]; //i

				Data[0][k] = r; Data[(21*featType)+6][k] = rvar; Data[(21*featType)+12][k] = rskw; 
				Data[(21*featType)+1][k] = g; Data[(21*featType)+7][k] = gvar; Data[(21*featType)+13][k] = gskw;
				Data[(21*featType)+2][k] = b; Data[(21*featType)+8][k] = bvar; Data[(21*featType)+14][k] = bskw;
				Data[(21*featType)+3][k] = rn; Data[(21*featType)+9][k] = rnvar; Data[(21*featType)+15][k] = rnskw;
				Data[(21*featType)+4][k] = gn; Data[(21*featType)+10][k] = gnvar; Data[(21*featType)+16][k] = gnskw;
				Data[(21*featType)+5][k] = bn; Data[(21*featType)+11][k] = bnvar; Data[(21*featType)+17][k] = bnskw;

				Data[(21*featType)+18][k] = i;
				Data[(21*featType)+19][k] = ivar;
				Data[(21*featType)+20][k] = iskw;
			}
			if (FEATSTEP>21)
			{
				int featType = 2;
				int tl = DataMask1[k];

				float count = mySeg->avgStore[tl*(FEATSTEP+1)+0];

				float r = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+1]; //r
				float g = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+2]; //g
				float b = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+3]; //b
				float rn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+4]; //rn
				float gn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+5]; //gb
				float bn = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+6]; //bn
				float i = mySeg->avgStore[tl*(FEATSTEP+1)+(featType*7)+7]; //i
				float rvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+0];
				float gvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+1];
				float bvar= mySeg->varStore[tl*FEATSTEP+(featType*7)+2];
				float rnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+3];
				float gnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+4];
				float bnvar = mySeg->varStore[tl*FEATSTEP+(featType*7)+5];
				float ivar = mySeg->varStore[tl*FEATSTEP+(featType*7)+6]; //i
				float rskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+0];
				float gskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+1];
				float bskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+2];
				float rnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+3];
				float gnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+4];
				float bnskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+5];
				float iskw = mySeg->skw1Store[tl*FEATSTEP+(featType*7)+6]; //i

				Data[0][k] = r; Data[(21*featType)+6][k] = rvar; Data[(21*featType)+12][k] = rskw; 
				Data[(21*featType)+1][k] = g; Data[(21*featType)+7][k] = gvar; Data[(21*featType)+13][k] = gskw;
				Data[(21*featType)+2][k] = b; Data[(21*featType)+8][k] = bvar; Data[(21*featType)+14][k] = bskw;
				Data[(21*featType)+3][k] = rn; Data[(21*featType)+9][k] = rnvar; Data[(21*featType)+15][k] = rnskw;
				Data[(21*featType)+4][k] = gn; Data[(21*featType)+10][k] = gnvar; Data[(21*featType)+16][k] = gnskw;
				Data[(21*featType)+5][k] = bn; Data[(21*featType)+11][k] = bnvar; Data[(21*featType)+17][k] = bnskw;

				Data[(21*featType)+18][k] = i;
				Data[(21*featType)+19][k] = ivar;
				Data[(21*featType)+20][k] = iskw;
			}
		});

		int * ol;
		ol = (int*)malloc(buffsize*sizeof(int));
		int * backup_ref_nb;
		backup_ref_nb = (int*)malloc(6*sizeof(int));
		memcpy(ol,overlayBuffer,buffsize*sizeof(int));
		memcpy(backup_ref_nb,ref_nb,6*sizeof(int));


		if (idx<0 || idx>=capsTrained)
		{
			//copy data into bucket
			BucketData[capsTrained] = Data;
	    

			//copy labels
			BucketLabel1[capsTrained] = Labels1;
			BucketLabel2[capsTrained] = Labels2;
			BucketLabel3[capsTrained] = Labels3;
			BucketLabel4[capsTrained] = Labels4;
			BucketLabel5[capsTrained] = Labels5;
			BucketLabel6[capsTrained] = Labels6;

		}
		else
		{
			//copy data into bucket

			BucketData[idx].clear();
			BucketLabel1[idx].clear();
			BucketLabel2[idx].clear();
			BucketLabel3[idx].clear();
			BucketLabel4[idx].clear();
			BucketLabel5[idx].clear();
			BucketLabel6[idx].clear();

			

			BucketData[idx].swap( Data);
	    

			//copy labels
			BucketLabel1[idx].swap(Labels1);
			BucketLabel2[idx].swap(Labels2);
			BucketLabel3[idx].swap(Labels3);
			BucketLabel4[idx].swap(Labels4);
			BucketLabel5[idx].swap(Labels5);
			BucketLabel6[idx].swap(Labels6);

		}
		//reshape bucket data to same as  Data(21, std::vector<float>(labelsSize));
		   //count size
		int total = 0;
		for(int i = 0; i < capsTrained+ (idx<0 || idx>=capsTrained) ? 1 : 0; i++){
			total += BucketData[i][0].size();
		}
		std::vector<std::vector<float>> ALLData(3*FEATSTEP, std::vector<float>(0));
		std::vector<int> ALLLabels1(0);
		std::vector<int> ALLLabels2(0);
		std::vector<int> ALLLabels3(0);
		std::vector<int> ALLLabels4(0);
		std::vector<int> ALLLabels5(0);
		std::vector<int> ALLLabels6(0);


		for(int i = 0; i < capsTrained+ (idx<0 || idx>=capsTrained) ? 1 : 0; i++){
			std::vector<std::vector<float>> dataSet = BucketData[i];
			std::vector<int> labelSet1 = BucketLabel1[i];
			std::vector<int> labelSet2 = BucketLabel2[i];
			std::vector<int> labelSet3 = BucketLabel3[i];
			std::vector<int> labelSet4 = BucketLabel4[i];
			std::vector<int> labelSet5 = BucketLabel5[i];
			std::vector<int> labelSet6 = BucketLabel6[i];

			ALLLabels1.insert(ALLLabels1.end(), labelSet1.begin(),labelSet1.end());
			ALLLabels2.insert(ALLLabels2.end(), labelSet2.begin(),labelSet2.end());
			ALLLabels3.insert(ALLLabels3.end(), labelSet3.begin(),labelSet3.end());
			ALLLabels4.insert(ALLLabels4.end(), labelSet4.begin(),labelSet4.end());
			ALLLabels5.insert(ALLLabels5.end(), labelSet5.begin(),labelSet5.end());
			ALLLabels6.insert(ALLLabels6.end(), labelSet6.begin(),labelSet6.end());


			for(int f = 0; f < 3*FEATSTEP;f++){
				std::vector<float> otherData = dataSet[f];
				ALLData[f].insert(ALLData[f].end(), otherData.begin(), otherData.end());
			}
		}

		Concurrency::parallel_for(int(0),(int)ALLLabels1.size(),[&](int k)
		{
			if (ALLLabels1.at(k)==1)
			{
				repClass[0] = true;
			}
			if (ALLLabels2.at(k)==1)
			{
				repClass[1] = true;
			}
			if (ALLLabels3.at(k)==1)
			{
				repClass[2] = true;
			}
			if (ALLLabels4.at(k)==1)
			{
				repClass[3] = true;
			}
			if (ALLLabels5.at(k)==1)
			{
				repClass[4] = true;
			}
			if (ALLLabels6.at(k)==1)
			{
				repClass[5] = true;
			}
		});
		

		/*
		int count = 0;
		int fsize = 0;
		for(int i = 0; i < bucketCount; i++){
			for(int f = 0; i < 21; f++){
				fsize  = BucketData[i][f].size();
				for(int s = 0; s < fsize; s++){
					ALLData[f][s + count] = BucketData[i][f][s];
				}
			}
			count += fsize;
		}
		*/

	  int Twidth = ALLData.size();
	  int Theight = ALLLabels1.size();

	//create strong classifer
     Classifier1 = Adaboost->RealAdaBoost(weak_learner1, ALLData, ALLLabels1, Twidth, Theight);//takes too long for editing
	 Classifier2 = Adaboost->RealAdaBoost(weak_learner2, ALLData, ALLLabels2, Twidth, Theight);//takes too long for editing
	 Classifier3 = Adaboost->RealAdaBoost(weak_learner3, ALLData, ALLLabels3, Twidth, Theight);//takes too long for editing
	 Classifier4 = Adaboost->RealAdaBoost(weak_learner4, ALLData, ALLLabels4, Twidth, Theight);//takes too long for editing
	 Classifier5 = Adaboost->RealAdaBoost(weak_learner5, ALLData, ALLLabels5, Twidth, Theight);//takes too long for editing
	 Classifier6 = Adaboost->RealAdaBoost(weak_learner6, ALLData, ALLLabels6, Twidth, Theight);//takes too long for editing

	 //update traincaps
	 if (idx<0 || idx>=capsTrained)
		 capsTrained ++;

}

void TADDengine::initClassifers(){
	///
	/// This sets up the variables needed for the AdaBoost classifiers and 
	/// creates six instances of the Real AdaBoost class.
		tree_node_w *weak_learner1 = new tree_node_w(6,100);
		tree_node_w *weak_learner2 = new tree_node_w(6,100);
		tree_node_w *weak_learner3 = new tree_node_w(6,100);
		tree_node_w *weak_learner4 = new tree_node_w(6,100);
		tree_node_w *weak_learner5 = new tree_node_w(6,100);
		tree_node_w *weak_learner6 = new tree_node_w(6,100);

		//reshape bucket data to same as  Data(21, std::vector<float>(labelsSize));
		   //count size
		int total = 0;
		for(int i = 0; i < capsTrained; i++){
			total += BucketData[i][0].size();
		}
		std::vector<std::vector<float>> ALLData(3*FEATSTEP, std::vector<float>(0));
		std::vector<int> ALLLabels1(0);
		std::vector<int> ALLLabels2(0);
		std::vector<int> ALLLabels3(0);
		std::vector<int> ALLLabels4(0);
		std::vector<int> ALLLabels5(0);
		std::vector<int> ALLLabels6(0);


		for(int i = 0; i < capsTrained; i++){
			std::vector<std::vector<float>> dataSet = BucketData[i];
			std::vector<int> labelSet1 = BucketLabel1[i];
			std::vector<int> labelSet2 = BucketLabel2[i];
			std::vector<int> labelSet3 = BucketLabel3[i];
			std::vector<int> labelSet4 = BucketLabel4[i];
			std::vector<int> labelSet5 = BucketLabel5[i];
			std::vector<int> labelSet6 = BucketLabel6[i];

			ALLLabels1.insert(ALLLabels1.end(), labelSet1.begin(),labelSet1.end());
			ALLLabels2.insert(ALLLabels2.end(), labelSet2.begin(),labelSet2.end());
			ALLLabels3.insert(ALLLabels3.end(), labelSet3.begin(),labelSet3.end());
			ALLLabels4.insert(ALLLabels4.end(), labelSet4.begin(),labelSet4.end());
			ALLLabels5.insert(ALLLabels5.end(), labelSet5.begin(),labelSet5.end());
			ALLLabels6.insert(ALLLabels6.end(), labelSet6.begin(),labelSet6.end());


			for(int f = 0; f < 3*FEATSTEP;f++){
				std::vector<float> otherData = dataSet[f];
				ALLData[f].insert(ALLData[f].end(), otherData.begin(), otherData.end());
			}
		}



		/*
		int count = 0;
		int fsize = 0;
		for(int i = 0; i < bucketCount; i++){
			for(int f = 0; i < 21; f++){
				fsize  = BucketData[i][f].size();
				for(int s = 0; s < fsize; s++){
					ALLData[f][s + count] = BucketData[i][f][s];
				}
			}
			count += fsize;
		}
		*/

	  int Twidth = ALLData.size();
	  int Theight = ALLLabels1.size();

	//create strong classifer
     Classifier1 = Adaboost->RealAdaBoost(weak_learner1, ALLData, ALLLabels1, Twidth, Theight);
	 Classifier2 = Adaboost->RealAdaBoost(weak_learner2, ALLData, ALLLabels2, Twidth, Theight);
	 Classifier3 = Adaboost->RealAdaBoost(weak_learner3, ALLData, ALLLabels3, Twidth, Theight);
	 Classifier4 = Adaboost->RealAdaBoost(weak_learner4, ALLData, ALLLabels4, Twidth, Theight);
	 Classifier5 = Adaboost->RealAdaBoost(weak_learner5, ALLData, ALLLabels5, Twidth, Theight);
	 Classifier6 = Adaboost->RealAdaBoost(weak_learner6, ALLData, ALLLabels6, Twidth, Theight);

}

void TADDengine::ClassParser( ){

	/*Mat img1(src1);
	Mat ch1, ch2, ch3;
	vector<Mat> channels(3);
// split img:
split(img1, channels);
// get the channels (dont forget they follow BGR order in OpenCV)
R = channels[0];
G = channels[1];
B = channels[2];
int rows = img1.rows;
int cols = img1.cols;

int i;
int j;

RED= cvCreateImage( cvSize( rows,cols ), IPL_DEPTH_8U, 1);
for (i=0; i<rows;i++){
for (j=0;j<cols;j++){
	if ( R.at<uchar>(i,j) = 255 && G.at<uchar>(i,j)==0 && B.uchar<i,j>==0) {RED.at<uchar>(i,j)=1};
                    }
                     }

GREEN= cvCreateImage( cvSize( rows,cols ), IPL_DEPTH_8U, 1);
for (i=0; i<rows;i++){
for (j=0;j<cols;j++){
	if ( R.at<uchar>(i,j) = 0 && G.at<uchar>(i,j)==255 && B.uchar<i,j>==0) {GREEN.at<uchar>(i,j)=1};
                    }
                     }*/



}

TADDengine::~TADDengine(){
	///
	///Destructor
	try
	{
	mySeg->~FastFeatures();
	//myClass->~FastKNN();
	Background->~Backseg();
	//SLIC
	cvReleaseImage(&imgBuffer4);
	cvReleaseMat(&zeros);

	//overlay
	cvReleaseImage(&cvOverlay);
	}
	catch (...)
	{
	}

	/*
	//classifyer
	for(int i = 0; i < ref_classes; i++)
	{
		free(ref[i]);
		free(dist_all[i]);
	}
	*/

	//free(ref);
	//free(dist_all);
}