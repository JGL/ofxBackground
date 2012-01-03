/*
 *  ofxBackgroundCvColorImage.cpp
 *  ofxBackground
 *
 *  Created by Joel Gethin Lewis on 16/03/2010.
 *
 */

#include "ofxBackgroundCvColorImage.h"
#include "ofxCvGrayscaleImage.h"
#include "ofxCvColorImage.h"
#include "ofxCvFloatImage.h"
#include "ofxCvShortImage.h"

	//--------------------------------------------------------------------------------
void ofxBackgroundCvColorImage::operator -= ( float value ) {
	cvSubS( cvImage, cvScalar(value, value, value), cvImageTemp );
	swapTemp();
    flagImageChanged();
}

	//--------------------------------------------------------------------------------
void ofxBackgroundCvColorImage::operator += ( float value ) {
	cvAddS( cvImage, cvScalar(value, value, value), cvImageTemp );
	swapTemp();
    flagImageChanged();
}

	//--------------------------------------------------------------------------------
void ofxBackgroundCvColorImage::operator = ( unsigned char* _pixels ) {
    setFromPixels( _pixels, width, height );
}

	//--------------------------------------------------------------------------------
void ofxBackgroundCvColorImage::operator = ( const ofxCvGrayscaleImage& _mom ) {
		// cast non-const,  no worries, we will reverse any chages
    ofxCvGrayscaleImage& mom = const_cast<ofxCvGrayscaleImage&>(_mom);
	if( matchingROI(getROI(), mom.getROI()) ) {
		cvCvtColor( mom.getCvImage(), cvImage, CV_GRAY2RGB );
        flagImageChanged();
	} else {
        ofLog(OF_LOG_ERROR, "in =, ROI mismatch");
	}
}

	//--------------------------------------------------------------------------------
void ofxBackgroundCvColorImage::operator = ( const ofxCvColorImage& _mom ) {
    if(this != &_mom) {  //check for self-assignment
						 // cast non-const,  no worries, we will reverse any chages
        ofxCvColorImage& mom = const_cast<ofxCvColorImage&>(_mom);
        if( matchingROI(getROI(), mom.getROI()) ) {
            cvCopy( mom.getCvImage(), cvImage, 0 );
            flagImageChanged();
        } else {
            ofLog(OF_LOG_ERROR, "in =, ROI mismatch");
        }
    } else {
        ofLog(OF_LOG_WARNING, "in =, you are assigning a ofxBackgroundCvColorImage to itself");
    }
}

	//--------------------------------------------------------------------------------
void ofxBackgroundCvColorImage::operator = ( const ofxCvFloatImage& _mom ) {
		// cast non-const,  no worries, we will reverse any chages
    ofxCvFloatImage& mom = const_cast<ofxCvFloatImage&>(_mom);
	if( matchingROI(getROI(), mom.getROI()) ) {
        if( cvGrayscaleImage == NULL ) {
            cvGrayscaleImage = cvCreateImage( cvSize(width,height), IPL_DEPTH_8U, 1 );
        }
        ofRectangle roi = getROI();
        setImageROI(cvGrayscaleImage, roi);
        rangeMap( mom.getCvImage(), cvGrayscaleImage,
				 mom.getNativeScaleMin(), mom.getNativeScaleMax(), 0, 255.0f );
		cvCvtColor( cvGrayscaleImage, cvImage, CV_GRAY2RGB );
        flagImageChanged();
	} else {
        ofLog(OF_LOG_ERROR, "in =, ROI mismatch");
	}
}

	//--------------------------------------------------------------------------------
void ofxBackgroundCvColorImage::operator = ( const ofxCvShortImage& _mom ) {
		// cast non-const,  no worries, we will reverse any chages
    ofxCvShortImage& mom = const_cast<ofxCvShortImage&>(_mom);
    if( matchingROI(getROI(), mom.getROI()) ) {
        if( cvGrayscaleImage == NULL ) {
            cvGrayscaleImage = cvCreateImage( cvSize(width,height), IPL_DEPTH_8U, 1 );
        }
        ofRectangle roi = getROI();
        setImageROI(cvGrayscaleImage, roi);
        rangeMap( mom.getCvImage(), cvGrayscaleImage, 0, 65535.0f, 0, 255.0f );
		cvCvtColor( cvGrayscaleImage, cvImage, CV_GRAY2RGB );
        flagImageChanged();
    } else {
        ofLog(OF_LOG_ERROR, "in =, ROI mismatch");
    }
}

	//--------------------------------------------------------------------------------
void ofxBackgroundCvColorImage::operator = ( const IplImage* _mom ) {
    ofxCvImage::operator = (_mom);
}