/*
 *  ofxBackground.h
 *
 *  Created by Joel Gethin Lewis on 15/03/2010.
 *	Version 0.03
 *
 *	Does nice background removal, using a variety of methods, all based on Chapter 9: Image Parts and Segmentation of "Learning OpenCV"
 * 
 *	Example code downloaded from: http://examples.oreilly.com/9780596516130/ comes with the following notice:
 *************** License:**************************
 Oct. 3, 2008
 Right to use this code in any way you want without warrenty, support or any guarentee of it working.
 
 BOOK: It would be nice if you cited it:
 Learning OpenCV: Computer Vision with the OpenCV Library
 by Gary Bradski and Adrian Kaehler
 Published by O'Reilly Media, October 3, 2008
 
 AVAILABLE AT: 
 http://www.amazon.com/Learning-OpenCV-Computer-Vision-Library/dp/0596516134
 Or: http://oreilly.com/catalog/9780596516130/
 ISBN-10: 0596516134 or: ISBN-13: 978-0596516130    
 
 OTHER OPENCV SITES:
 * The source code is on sourceforge at:
 http://sourceforge.net/projects/opencvlibrary/
 * The OpenCV wiki page (As of Oct 1, 2008 this is down for changing over servers, but should come back):
 http://opencvlibrary.sourceforge.net/
 * An active user group is at:
 http://tech.groups.yahoo.com/group/OpenCV/
 * The minutes of weekly OpenCV development meetings are at:
 http://pr.willowgarage.com/wiki/OpenCV
 **************************************************
 *
 *	Template for class was ofxContourFinder from ofxOpenCV. Thanks all of the OF Community, OF-DEV, YesYesNo, Todd Vanderlin, Kyle McDonald and 
 *	Chris O'Shea
 */

#ifndef OFX_BACKGROUND
#define OFX_BACKGROUND

#include "ofxCvConstants.h"
#include "ofxBackgroundCvColorImage.h"
#include "ofxCvGrayScaleImage.h"

#include "cv_yuv_codebook.h"

	//from ch9_AvgBackground.h

#define NUM_CAMERAS   1      		//This function can handle an array of cameras
#define HIGH_SCALE_NUM 7.0        	//How many average differences from average image on the high side == background
#define LOW_SCALE_NUM 6.0		//How many average differences from average image on the low side == background

#define LEARNING_TIME 10000 //ten seconds of learning time, then switch

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Accumulate average and ~std (really absolute difference) image and use this to detect background and foreground
	//
	// Typical way of using this is to:
	// 	AllocateImages();
	//	//loop for N images to accumulate background differences
	//	accumulateBackground();
	//	//When done, turn this into our avg and std model with high and low bounds
	//	createModelsfromStats();
	//	//Then use the function to return background in a mask (255 == foreground, 0 == background)
	//	backgroundDiff(IplImage *I,IplImage *Imask, int num);
	//	//Then tune the high and low difference from average image background acceptance thresholds
	//	float scalehigh,scalelow; //Set these, defaults are 7 and 6. Note: scalelow is how many average differences below average
	//	scaleHigh(scalehigh);
	//	scaleLow(scalelow);
	//	//That is, change the scale high and low bounds for what should be background to make it work.
	//	//Then continue detecting foreground in the mask image
	//	backgroundDiff(IplImage *I,IplImage *Imask, int num);
	//
	//NOTES: num is camera number which varies from 0 ... NUM_CAMERAS - 1.  Typically you only have one camera, but this routine allows
	//     	 you to index many.
	//

class ofxBackground : public ofBaseDraws {
	
public:

    ofxBackground();
    virtual  ~ofxBackground();
    
	virtual float getWidth() { return _width; };    //set after update call
	virtual float getHeight() { return _height; };  //set after update call
	
    virtual void  allocate( int w, int h );
    virtual void  clear();
	virtual void  reset(int w, int h);
	
	virtual void allocateImages(int w, int h);
    
    virtual void update(ofxCvColorImage& input); //update the background image funky

	virtual void deallocateImages();
	virtual void accumulateBackground(IplImage *I);
	virtual void scaleHigh(float scale = HIGH_SCALE_NUM);
	virtual void scaleLow(float scale = LOW_SCALE_NUM);
	virtual void createModelsfromStats();
	virtual void backgroundDiff(IplImage *I,IplImage *Imask);
	
    virtual void  draw() { draw(0,0, _width, _height); };
    virtual void  draw( float x, float y ) { draw(x,y, _width, _height); };
    virtual void  draw( float x, float y, float w, float h );
	virtual void setAnchorPercent(float xPct, float yPct);
    virtual void setAnchorPoint(int x, int y);
	virtual void resetAnchor();
	
	virtual void startLearning(); //reset the timeStartedLearning float to start the learning process again

		//bits to make it easier to draw, and to get the information out - allocate first! 
	ofxCvGrayscaleImage backgroundAverage;
	ofxCvGrayscaleImage backgroundAverageConnectedComponents;
    ofxCvGrayscaleImage backgroundCodebook;
	ofxCvGrayscaleImage backgroundCodeBookConnectedComponents;	
	
protected:
	
    int  _width;
    int  _height;
	bool bAllocated;
	
    ofxCvColorImage     inputCopy;
	ofxBackgroundCvColorImage	yuvImage; //yuvImage is for codebook method
    
    ofPoint  anchor;
    bool  bAnchorIsPct;      
	
		//all from Chapter 9, Learning OpenCV, ch9_AvgBackground.h
		
	IplImage* IavgF;
	IplImage* IdiffF;
	IplImage* IprevF;
	IplImage* IhiF;
	IplImage* IlowF;
	IplImage* Iscratch;
	IplImage* Iscratch2;
	IplImage* Igray1;
	IplImage* Igray2;
	IplImage* Igray3;
	IplImage* Imaskt;
	IplImage* Ilow1;
	IplImage* Ilow2;
	IplImage* Ilow3;
	IplImage* Ihi1;
	IplImage* Ihi2;
	IplImage* Ihi3;
	
	float Icount;
	
		//all from Chapter 9, Learning OpenCV, ch9_backgroundAVG.cpp
	
		//VARIABLES for CODEBOOK METHOD:
	codeBook *cB;   //This will be our linear model of the image, a vector 
					//of lengh = height*width
	int maxMod[CHANNELS];	//Add these (possibly negative) number onto max 
							// level when code_element determining if new pixel is foreground
	int minMod[CHANNELS]; 	//Subract these (possible negative) number from min 
							//level code_element when determining if pixel is foreground
	unsigned cbBounds[CHANNELS]; //Code Book bounds for learning
	bool ch[CHANNELS];		//This sets what channels should be adjusted for background bounds
	int nChannels;
	int imageLen;
	uchar *pColor; //YUV pointer	
	
		//all in the main of same
	
    IplImage* ImaskAVG;
	IplImage* ImaskAVGCC;
    IplImage* ImaskCodeBook;
	IplImage* ImaskCodeBookCC;
	
	float scalehigh, scalelow;
	
	float timeStartedLearning;
	
	bool bStatsDone;
	
	bool bLearning;
};

#endif
