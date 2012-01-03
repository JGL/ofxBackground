#include "testApp.h"


//--------------------------------------------------------------
void testApp::setup(){
    
	vidGrabber.setVerbose(true);
	vidGrabber.initGrabber(320,240);
    
    colorImg.allocate(320,240);
    
	bLearnBackground = true;	
	
	backgroundAddon.allocate(320, 240);
}

//--------------------------------------------------------------
void testApp::update(){
	ofBackground(100,100,100);
    
    bool bNewFrame = false;
    
	vidGrabber.grabFrame();
	bNewFrame = vidGrabber.isFrameNew();
    
	if (bNewFrame){
        
		colorImg.setFromPixels(vidGrabber.getPixels(), 320,240);
		
		backgroundAddon.update(colorImg);
        
	}
	
	if(bLearnBackground){
		backgroundAddon.startLearning();
		bLearnBackground = false;
	}
    
}

//--------------------------------------------------------------
void testApp::draw(){
	ofSetColor(0xffffff);
	colorImg.draw(20,20);
    
	backgroundAddon.draw(20+320+20, 20); //draw it to the side
}


//--------------------------------------------------------------
void testApp::keyPressed  (int key){
    
	switch (key){
		case 'j':
			bLearnBackground = true;
			break;
	}
}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
    
}


//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
    
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
    
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
    
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){ 
    
}

