/*
 *  ofxBackground.cpp
 *
 *  Created by Joel Gethin Lewis on 15/03/2010.
 *
 */

#include "ofxBackground.h"

ofxBackgroundLearningCompleteEvent ofxBackground::onLearningComplete;

	//--------------------------------------------------------------------------------
ofxBackground::ofxBackground() {
    _width = 0;
    _height = 0;
	
	nChannels = CHANNELS;
	imageLen = 0;
	
    ImaskAVG = 0, ImaskAVGCC = 0;
    ImaskCodeBook = 0, ImaskCodeBookCC = 0;
	
	maxMod[0] = 3;  //Set color thresholds to default values
	minMod[0] = 10;
	maxMod[1] = 1;
	minMod[1] = 1;
	maxMod[2] = 1;
	minMod[2] = 1;
	scalehigh = HIGH_SCALE_NUM;
	scalelow = LOW_SCALE_NUM;	
	
	bAllocated = false;
	
	timeStartedLearning = 0.f;
	
	bLearning = false;
}

	//--------------------------------------------------------------------------------
ofxBackground::~ofxBackground() {
	clear();
	deallocateImages();	//be good
}

	//--------------------------------------------------------------------------------
void ofxBackground::allocate( int w, int h ) {
	if (bAllocated == true){
		ofLog(OF_LOG_WARNING, "in allocate, reallocating a ofxCvImage, within OfxBackground");
		clear();
	}
	
	inputCopy.allocate(w, h);
	yuvImage.allocate(w, h);
	
	backgroundAverage.allocate(w, h);
	backgroundAverageConnectedComponents.allocate(w, h);
    backgroundCodebook.allocate(w, h);
	backgroundCodeBookConnectedComponents.allocate(w, h);
	
		//AVG METHOD ALLOCATION
	allocateImages(w,h); //redo everything if you change the size! and this will be triggered first time round
	scaleHigh(scalehigh);
	scaleLow(scalelow);
	ImaskAVG = cvCreateImage( cvGetSize(inputCopy.getCvImage()), IPL_DEPTH_8U, 1 );
	ImaskAVGCC = cvCreateImage( cvGetSize(inputCopy.getCvImage()), IPL_DEPTH_8U, 1 );
	cvSet(ImaskAVG,cvScalar(255));
		//CODEBOOK METHOD ALLOCATION:
	yuvImage = cvCloneImage(inputCopy.getCvImage());
	ImaskCodeBook = cvCreateImage( cvGetSize(inputCopy.getCvImage()), IPL_DEPTH_8U, 1 );
	ImaskCodeBookCC = cvCreateImage( cvGetSize(inputCopy.getCvImage()), IPL_DEPTH_8U, 1 );
	cvSet(ImaskCodeBook,cvScalar(255));
	imageLen = inputCopy.getCvImage()->width*inputCopy.getCvImage()->height;
	cB = new codeBook [imageLen];
	for(int f = 0; f<imageLen; f++)
	{
		cB[f].numEntries = 0;
	}
	for(int nc=0; nc<nChannels;nc++)
	{
		cbBounds[nc] = 10; //Learning bounds factor
	}
	ch[0] = true; //Allow threshold setting simultaneously for all channels
	ch[1] = true;
	ch[2] = true;		
	
	_width = w;
	_height = h;
	bAllocated = true;
	
	timeStartedLearning = ofGetElapsedTimeMillis(); //for safetly? TODO: question
	bStatsDone = false;
}

	//--------------------------------------------------------------------------------
void ofxBackground::clear() {
	
	if (bAllocated == true){
		inputCopy.clear();
		yuvImage.clear(); //yuvImage is for codebook method
		
		backgroundAverage.clear();
		backgroundAverageConnectedComponents.clear();
		backgroundCodebook.clear();
		backgroundCodeBookConnectedComponents.clear();	
		
		bAllocated = false;
	}
}

void ofxBackground::reset(int w, int h) {
	clear();
	deallocateImages();	//be good	
	allocate(w,h);
}

void ofxBackground::allocateImages(int w, int h){
	
	IavgF = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 3 );
	IdiffF = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 3 );
	IprevF = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 3 );
	IhiF = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 3 );
	IlowF = cvCreateImage(cvSize(w,h), IPL_DEPTH_32F, 3 );
	Ilow1 = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 1 );
	Ilow2 = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 1 );
	Ilow3 = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 1 );
	Ihi1 = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 1 );
	Ihi2 = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 1 );
	Ihi3 = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 1 );
	cvZero(IavgF);
	cvZero(IdiffF);
	cvZero(IprevF);
	cvZero(IhiF);
	cvZero(IlowF);		
	Icount = 0.00001; //Protect against divide by zero

	Iscratch = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 3 );
	Iscratch2 = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 3 );
	Igray1 = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 1 );
	Igray2 = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 1 );
	Igray3 = cvCreateImage( cvSize(w,h), IPL_DEPTH_32F, 1 );
	Imaskt = cvCreateImage( cvSize(w,h), IPL_DEPTH_8U, 1 );
	
	cvZero(Iscratch);
	cvZero(Iscratch2);
}

void ofxBackground::update(ofxCvColorImage& input){
	float now = ofGetElapsedTimeMillis();
	
		// get width/height disregarding ROI
    IplImage* ipltemp = input.getCvImage();
    _width = ipltemp->width;
    _height = ipltemp->height;
	
	if( inputCopy.getWidth() == 0 ) {
		allocate( _width, _height );
	} else if( inputCopy.getWidth() != _width || inputCopy.getHeight() != _height ) {
			// reallocate to new size
		clear();
		allocate( _width, _height );
	} else { //don't do anything unless we have allocated! (and therefore set timeStartedLearning to a safe, non zero value)
		
		inputCopy = input;
		inputCopy.setROI( input.getROI() );
		yuvImage.setROI( input.getROI() ); //pass on ROI'ness
		
		yuvImage.setFromPixels(inputCopy.getPixels(), _width, _height);
		yuvImage.convertRgbToYuv();	
		
		if((now-timeStartedLearning) < LEARNING_TIME){
				//then we should be learning
				//LEARNING THE AVERAGE AND AVG DIFF BACKGROUND
			accumulateBackground(inputCopy.getCvImage());
				//LEARNING THE CODEBOOK BACKGROUND
			pColor = (uchar *)((yuvImage.getCvImage())->imageData);
			for(int c=0; c<imageLen; c++)
			{
				cvupdateCodeBook(pColor, cB[c], cbBounds, nChannels);
				pColor += 3;
			}
			
				//TODO: clear stale entries
			
			bStatsDone = false;
			bLearning = true;
		}else {
				//its either time to do stats or not
			bLearning = false;
			if(!bStatsDone){
					//do the stats, just the once
				createModelsfromStats(); //create the background model
                ofNotifyEvent(ofxBackgroundLearningCompleteEvent::events, onLearningComplete);
                
				bStatsDone = true;
			}else {
					//learn as normal, find the foreground if any
						//FIND FOREGROUND BY AVG METHOD:
					backgroundDiff(inputCopy.getCvImage(),ImaskAVG);
					cvCopy(ImaskAVG,ImaskAVGCC);
					cvconnectedComponents(ImaskAVGCC);
						//FIND FOREGROUND BY CODEBOOK METHOD
					uchar maskPixelCodeBook;
					pColor = (uchar *)((yuvImage.getCvImage())->imageData); //3 channel yuv image
					uchar *pMask = (uchar *)((ImaskCodeBook)->imageData); //1 channel image
					for(int c=0; c<imageLen; c++)
					{
						maskPixelCodeBook = cvbackgroundDiff(pColor, cB[c], nChannels, minMod, maxMod);
						*pMask++ = maskPixelCodeBook;
						pColor += 3;
					}
						//This part just to visualize bounding boxes and centers if desired
					cvCopy(ImaskCodeBook,ImaskCodeBookCC);	
					cvconnectedComponents(ImaskCodeBookCC);				
				
					//TODO: update the learned background pixels....
					//TODO: clear stale codebook entries on a much slower frequency
			}

		}
		
		backgroundAverage = ImaskAVG;
		backgroundAverageConnectedComponents = ImaskAVGCC;
		backgroundCodebook = ImaskCodeBook;
		backgroundCodeBookConnectedComponents = ImaskCodeBookCC;	
	}
}

void ofxBackground::deallocateImages()
{
	cvReleaseImage(&IavgF);
	cvReleaseImage(&IdiffF);
	cvReleaseImage(&IprevF);
	cvReleaseImage(&IhiF);
	cvReleaseImage(&IlowF);
	cvReleaseImage(&Ilow1);
	cvReleaseImage(&Ilow2);
	cvReleaseImage(&Ilow3);
	cvReleaseImage(&Ihi1);
	cvReleaseImage(&Ihi2);
	cvReleaseImage(&Ihi3);
	cvReleaseImage(&Iscratch);
	cvReleaseImage(&Iscratch2);
	
	cvReleaseImage(&Igray1  );
	cvReleaseImage(&Igray2 );
	cvReleaseImage(&Igray3 );
	
	cvReleaseImage(&Imaskt);
	
	cvReleaseImage(&ImaskAVG);
	cvReleaseImage(&ImaskAVGCC);
	cvReleaseImage(&ImaskCodeBook);
	cvReleaseImage(&ImaskCodeBookCC);
}

	// Accumulate the background statistics for one more frame
	// We accumulate the images, the image differences and the count of images for the 
	//    the routine createModelsfromStats() to work on after we're done accumulating N frames.
	// I		Background image, 3 channel, 8u
	// number	Camera number
void ofxBackground::accumulateBackground(IplImage *I)
{
	static int first = 1;
	cvCvtScale(I,Iscratch,1,0); //To float;
	if (!first){
		cvAcc(Iscratch,IavgF);
		cvAbsDiff(Iscratch,IprevF,Iscratch2);
		cvAcc(Iscratch2,IdiffF);
		Icount += 1.0;
	}
	first = 0;
	cvCopy(Iscratch,IprevF);
}

	// Scale the average difference from the average image high acceptance threshold
void ofxBackground::scaleHigh(float scale)
{
	cvConvertScale(IdiffF,Iscratch,scale); //Converts with rounding and saturation
	cvAdd(Iscratch,IavgF,IhiF);
	cvCvtPixToPlane( IhiF, Ihi1,Ihi2,Ihi3, 0 );
}

	// Scale the average difference from the average image low acceptance threshold
void ofxBackground::scaleLow(float scale)
{
	cvConvertScale(IdiffF,Iscratch,scale); //Converts with rounding and saturation
	cvSub(IavgF,Iscratch,IlowF);
	cvCvtPixToPlane( IlowF, Ilow1,Ilow2, Ilow3, 0 );
}

	//Once you've learned the background long enough, turn it into a background model
void ofxBackground::createModelsfromStats()
{
	cvConvertScale(IavgF,IavgF,(double)(1.0/Icount));
	cvConvertScale(IdiffF,IdiffF,(double)(1.0/Icount));
	cvAddS(IdiffF,cvScalar(1.0,1.0,1.0),IdiffF);  //Make sure diff is always something
	scaleHigh(HIGH_SCALE_NUM);
	scaleLow(LOW_SCALE_NUM);
}

	// Create a binary: 0,255 mask where 255 means forground pixel
	// I		Input image, 3 channel, 8u
	// Imask	mask image to be created, 1 channel 8u
	// num		camera number.
	//
void ofxBackground::backgroundDiff(IplImage *I,IplImage *Imask)  //Mask should be grayscale
{
	cvCvtScale(I,Iscratch,1,0); //To float;
								//Channel 1
	cvCvtPixToPlane( Iscratch, Igray1,Igray2,Igray3, 0 );
	cvInRange(Igray1,Ilow1,Ihi1,Imask);
		//Channel 2
	cvInRange(Igray2,Ilow2,Ihi2,Imaskt);
	cvOr(Imask,Imaskt,Imask);
		//Channel 3
	cvInRange(Igray3,Ilow3,Ihi3,Imaskt);
	cvOr(Imask,Imaskt,Imask);
		//Finally, invert the results
	cvSubRS( Imask, cvScalar(255), Imask);
}

	//--------------------------------------------------------------------------------
void ofxBackground::draw( float x, float y, float w, float h ) {
	
	float now = ofGetElapsedTimeMillis();
	
    float scalex = 0.0f;
    float scaley = 0.0f;
    if( _width != 0 ) { scalex = w/_width; } else { scalex = 1.0f; }
    if( _height != 0 ) { scaley = h/_height; } else { scaley = 1.0f; }
	
    if(bAnchorIsPct){
        x -= anchor.x * w;
        y -= anchor.y * h;
    }else{
        x -= anchor.x;
        y -= anchor.y;
    }
	
		// ---------------------------- draw the various masks
	ofSetColor(0xFFFFFF);
    glPushMatrix();
    glTranslatef( x, y, 0.0 );
    glScalef( scalex, scaley, 0.0 );
	
	if (bLearning) {
		backgroundAverage.draw(0,0);
		ofDrawBitmapString("Average background: LEARNING", 0, _height+10);
		backgroundAverageConnectedComponents.draw(0, 20+_height);
		ofDrawBitmapString("Average Connected Components: LEARNING", 0, _height+20+_height+10);
		backgroundCodebook.draw(_width+20, 0);
		ofDrawBitmapString("Codebook: LEARNING", _width+20, _height+10);
		backgroundCodeBookConnectedComponents.draw(_width+20, 20+_height);
		ofDrawBitmapString("Codebook Connected Components: LEARNING", _width+20, _height+20+_height+10);
		string timeLeft = ofToString(LEARNING_TIME-(now-timeStartedLearning));
		ofDrawBitmapString("Learning Time Left: "+timeLeft, 0, _height+20+_height+10+10);
	}else{
		backgroundAverage.draw(0,0);
		ofDrawBitmapString("Average background", 0, _height+10);
		backgroundAverageConnectedComponents.draw(0, 20+_height);
		ofDrawBitmapString("Average Connected Components", 0, _height+20+_height+10);
		backgroundCodebook.draw(_width+20, 0);
		ofDrawBitmapString("Codebook", _width+20, _height+10);
		backgroundCodeBookConnectedComponents.draw(_width+20, 20+_height);
		ofDrawBitmapString("Codebook Connected Components", _width+20, _height+20+_height+10);
		
			//old stuff from ofxCvCountourFinder
			//	ofNoFill();
			//	for( int i=0; i<(int)blobs.size(); i++ ) {
			//		ofRect( blobs[i].boundingRect.x, blobs[i].boundingRect.y,
			//			   blobs[i].boundingRect.width, blobs[i].boundingRect.height );
			//	}
			//	
			//		// ---------------------------- draw the blobs
			//	ofSetColor(0x00FFFF);
			//	
			//	for( int i=0; i<(int)blobs.size(); i++ ) {
			//		ofNoFill();
			//		ofBeginShape();
			//		for( int j=0; j<blobs[i].nPts; j++ ) {
			//			ofVertex( blobs[i].pts[j].x, blobs[i].pts[j].y );
			//		}
			//		ofEndShape();
			//		
	}	
	glPopMatrix();
}

	//--------------------------------------------------------------------------------
void ofxBackground::setAnchorPercent( float xPct, float yPct ){
    anchor.x = xPct;
    anchor.y = yPct;
    bAnchorIsPct = true;
}

	//--------------------------------------------------------------------------------
void ofxBackground::setAnchorPoint( int x, int y ){
    anchor.x = x;
    anchor.y = y;
    bAnchorIsPct = false;
}

	//--------------------------------------------------------------------------------
void ofxBackground::resetAnchor(){
    anchor.set(0,0);
    bAnchorIsPct = false;
}

void ofxBackground::startLearning(){
	timeStartedLearning = ofGetElapsedTimeMillis();
}
