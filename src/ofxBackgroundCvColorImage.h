/*
 *  ofxBackgroundCvColorImage.h
 *  ofxBackground
 *
 *  Created by Joel Gethin Lewis on 16/03/2010.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef OFX_BACKGROUND_CV_COLOR_IMAGE
#define OFX_BACKGROUND_CV_COLOR_IMAGE

#include "ofxCvColorImage.h"

class ofxBackgroundCvColorImage : public ofxCvColorImage {
public:
	virtual void  convertRgbToYuv(){
			cvCvtColor( cvImage, cvImageTemp, CV_BGR2YCrCb);
			swapTemp();
			flagImageChanged();
	}; //jgl add
	
public: //need all the assignment, see Chaper 13 "Using inheritance and dynamic binding" "Accelerated C++"
	
    virtual void  operator -= ( float value );
    virtual void  operator += ( float value ); 
	
    virtual void  operator = ( unsigned char* _pixels );
    virtual void  operator = ( const ofxCvGrayscaleImage& mom );
    virtual void  operator = ( const ofxCvColorImage& mom );
    virtual void  operator = ( const ofxCvFloatImage& mom );
    virtual void  operator = ( const ofxCvShortImage& mom );
    virtual void  operator = ( const IplImage* mom );    
    
		// virtual void  operator -= ( ofxCvImage& mom );                         //in base class 
		// virtual void  operator += ( ofxCvImage& mom );                         //in base class 
    using ofxCvImage::operator+=;  //force inheritance 
    using ofxCvImage::operator-=;  //force inheritance    
								   // virtual void  operator *= ( ofxCvImage& mom );                         //in base class 
								   // virtual void  operator &= ( ofxCvImage& mom );                         //in base class     
};
#endif