#include "testApp.h"

//--------------------------------------------------------------
void testApp::setup(){
    
    bUseWarp = true;
    
    // load a movie for testing purposes
    vid.loadMovie("pathtoyourmovie.mov");
    vid.play();
    
    // allocate the warp with width, height, numXControlPoints,  
    // numYControlPoints, and the resolution of grid divisions in pixels
    
    // NB: the bezier warp needs to use only power of 2 sizes internally
    // but can use any width/height passed during allocation (internally
    // we scale the fbo we are warping) - so it's good to sometimes use
    // sizes a little smaller than normal if efficieny is your thing
    // eg., 1920 x 1080 will force the use of 2048 x 2048 internal fbo
    // but an 1820 x 1024 size will only use a 2048 x 1024 internal fbo ;)
    
    warp.allocate(ofGetWidth(), ofGetHeight(), 5, 4, 80);
    
    ofBackground(0, 0, 0);
}

//--------------------------------------------------------------
void testApp::update(){
    
    vid.update();
    
    if(bUseWarp){
        
        // use the bezier warp like an fbo by calling 
        // begin() and end() around anything you want warped
        
        warp.begin();
        {
            vid.draw(0, 0, ofGetWidth(), ofGetHeight());
        }
        warp.end();
        
    }
}

//--------------------------------------------------------------
void testApp::draw(){
    
    if(bUseWarp){
        // draw using the bezier warp
        warp.draw();
    }else{
        // draw the video without the warp
        vid.draw(0, 0, ofGetWidth(), ofGetHeight());
    }
    
    // show some info
    ostringstream os;
    os << "Press 'w' to toggle using warp" << endl;
    os << "Press 'r' to reset the warp grid" << endl;
    os << "Press 's' to show the warp grid" << endl;
    os << "Use arrow keys to increase/decrease number of warp control points" << endl;
    os << endl;
    os << "FPS: " << ofGetFrameRate();
    ofDrawBitmapString(os.str(), 20, 20);
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
    switch (key) {
        case 'w':
            bUseWarp = !bUseWarp;
            break;
        case 'r':
            warp.resetWarpGrid();
            break;
        case 's':
            warp.toggleShowWarpGrid();
            break;
        case 'p':
            warp.setWarpGridPosition(100, 100, (ofGetWidth() - 200), (ofGetHeight() - 200));
            break;
        case OF_KEY_UP:
            warp.setWarpGrid(warp.getNumXPoints(), warp.getNumYPoints() + 1);
            break;
        case OF_KEY_RIGHT:
            warp.setWarpGrid(warp.getNumXPoints() + 1, warp.getNumYPoints());
            break;
        case OF_KEY_DOWN:
            warp.setWarpGrid(warp.getNumXPoints(), warp.getNumYPoints() - 1);
            break;
        case OF_KEY_LEFT:
            warp.setWarpGrid(warp.getNumXPoints() - 1, warp.getNumYPoints());
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