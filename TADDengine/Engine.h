#ifndef Engine_H
#define Engine_H

#include <QtGui/QMouseEvent>
#include <QtCore\qfile.h>
//#include <QtGui\qapplication.h>
#include <vector>
#include <QtGui/QMessageBox>
//#include "../cKNN/FastKNN.h"
#include "../cSLIC/FastFeatures.h"
#include "../multiAdaboost/adaboost.h"
#include "../BackgroundSeg/Backseg.h"

//#include "../adaboost/adaboost.h"

#include <tbb\parallel_for.h>
#include <tbb\parallel_invoke.h>

#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <fstream>


class TADDengine
{

private:    

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive & ar,
            const unsigned int version) {
		ar & BOOST_SERIALIZATION_NVP(width);
		ar & BOOST_SERIALIZATION_NVP(height);
        ar & BOOST_SERIALIZATION_NVP(capsTrained);
		
		ar & boost::serialization::make_nvp("BucketData",BucketData);

		ar & boost::serialization::make_nvp("BucketLabel1",BucketLabel1);
		ar & boost::serialization::make_nvp("BucketLabel2",BucketLabel2);
		ar & boost::serialization::make_nvp("BucketLabel3",BucketLabel3);
		ar & boost::serialization::make_nvp("BucketLabel4",BucketLabel4);
		ar & boost::serialization::make_nvp("BucketLabel5",BucketLabel5);
		ar & boost::serialization::make_nvp("BucketLabel6",BucketLabel6);
		
		ar & boost::serialization::make_nvp("BucketOverlay",BucketOverlay);
		ar & boost::serialization::make_nvp("BucketCounts",BucketCounts);
		ar & boost::serialization::make_nvp("colourList",colourList);
		
        ar & BOOST_SERIALIZATION_NVP(repClass);
    }

	QMessageBox msgBox;

private:
	// debug stuff
	IplImage *dbgIplImg;
	unsigned char *debugimg;

	//all
	int width, height, nChannels, step, depth;
	int size; //ammount of superpixels in soruce image
	uchar *cvData;

	//SLIC
	int sPixels;
	unsigned char* imgBuffer;
	unsigned char* threshBuffer;
	IplImage* imgBuffer4;
	CvMat * zeros;
	SEGMETHOD segMethod;
	float segWeight;

	//cvOverlay
	IplImage* cvOverlay;
	IplImage *RED;
	IplImage *GREEN;
	IplImage *BLUE;
	IplImage *YELLOW;
	IplImage *PURPLE;
	IplImage *ORANGE;
	//int* labelMask;
	int mskIndex;
	int selectedLabel;
	int* overlayBuffer;

	int buffsize;
	int backupIdx;
	QList<int*> unredo;
	QList<int*> unredo_ref_nb;

	//Classifer
	float *query;         
	float **ref;
	float** dist_all; 

	int *ind;                 
	int query_nb;
	int  dim;
    int k;
    int ref_classes; 

	//adaboost
	tuple<vector<tree_node*>,vector<float>,vector<float>> Classifier1;
	tuple<vector<tree_node*>,vector<float>,vector<float>> Classifier2;
	tuple<vector<tree_node*>,vector<float>,vector<float>> Classifier3;
	tuple<vector<tree_node*>,vector<float>,vector<float>> Classifier4;
	tuple<vector<tree_node*>,vector<float>,vector<float>> Classifier5;
	tuple<vector<tree_node*>,vector<float>,vector<float>> Classifier6;
	vector<int> signV;

	int* labelClass; //result
	
public:
	//hack for script
	int* labelMask;

	TADDengine();
	void initLoad(bool preserve = false);
	void loadClassifier(bool preserve = false);//classifer loader
	void initClassifers();
	int loadSettings(QString fname);

	TADDengine(int width, int height);
	~TADDengine();
	IplImage* SLIC(IplImage* src, int draw);
	 
    //overlay
	IplImage* initOverlay(); //initalize overlay
	IplImage* clearOverlay();
	IplImage* updateOverlay(QPoint clickPos, int classLabel);
	IplImage* overlayFromBucket(int bucketIdx);

	//classify overlay
	//IplImage* classifyOverlayKNN(bool isClassify); 
	IplImage* classifyOverlayAdaboost(bool isClassify); 
	std::vector<int> Matches;
	std::vector<int> MatchesBG;
	//train
	void refKNN();
	void refAdaboost(int idx=-1);

	//background
	void trainBackground(IplImage* src);
	void updateMatches(IplImage* src, IplImage* threshold);
	IplImage* removeBackground(IplImage* src);
	IplImage* UpdateBackground(QPoint clickPos, IplImage* src, int Value);

    int *ref_nb; 
	int *sign_nb;

	int capsTrained;

	void KeepBackup();
	IplImage * Undo();
	IplImage * Redo();
	void ClassParser();

private:
	void generateOverlay();
	void RetriveProperties(IplImage *cvOverlay);
	//void generateOverlayClass();
	void generateOverlayClassBoosting();

private:
	FastFeatures* mySeg;
	//FastKNN* myClass;
	Boosting* Adaboost;
	Backseg* Background;

public:
	std::vector<std::vector<std::vector<float>>> BucketData;
	std::vector<std::vector<int>> BucketOverlay;
	std::vector<std::vector<int>> BucketCounts;
	
	std::vector<std::vector<int>> BucketLabel1;
	std::vector<std::vector<int>> BucketLabel2;
	std::vector<std::vector<int>> BucketLabel3;
	std::vector<std::vector<int>> BucketLabel4;
	std::vector<std::vector<int>> BucketLabel5;
	std::vector<std::vector<int>> BucketLabel6;

	std::vector<std::vector<int>> colourList;

	int bucketCount;
	
	bool repClass[6];

};

#endif