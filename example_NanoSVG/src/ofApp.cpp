#include "ofApp.h"


//static NSVGimage* image;

//--------------------------------------------------------------
void ofApp::setup(){
    
    
    string path = "svgs/";
    ofDirectory dir(path);
    dir.allowExt("svg");
    dir.listDir();
    dir.sort();
    
    // and load them all
    files = dir.getFiles();
    
    dir.close();
//
//    cout << ofToDataPath("svgs/seblee.svg") << endl;
////    // Load SVG
//
//    image = nsvgParseFromFile(ofToDataPath("svgs/3_seblee2.svg").c_str(), "px", 96);
//    printf("size: %f x %f\n", image->width, image->height);
////    // Use...

    
//    // Delete
//    nsvgDelete(image);
    
}

//--------------------------------------------------------------
void ofApp::update(){

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofBackground(10);
    
    ofxNanoSvg svg;
    svg.load(ofToDataPath(files[currentSvgIndex]));
    svg.draw();
    
    /*
    image = nsvgParseFromFile(ofToDataPath(files[currentSvgIndex]).c_str(), "px", 96);

    ofNoFill();
    int numshapes = 0;
    for (NSVGshape *shape = image->shapes; shape != NULL; shape = shape->next) {
        numshapes++;
    }
    int maxcount = numshapes; // ofMap(ofGetMouseX(), 0, ofGetWidth(), 0, numshapes);
    int count = 0;
    for (NSVGshape *shape = image->shapes; (shape != NULL) && (count<maxcount); shape = shape->next) {
        count++;
        if(!(shape->flags&=NSVG_FLAGS_VISIBLE)) continue;
        if(count == maxcount) {
            //cout << shape->opacity << " " << ofToHex(shape->stroke.color) << " " << ofToHex(shape->fill.color) << endl;
        }
       // ofSetLineWidth(shape->strokeWidth);
       
        for (NSVGpath *path = shape->paths; path != NULL; path = path->next) {
            for (int i = 0; i < path->npts-1; i += 3) {
                float* p = &path->pts[i*2];
                
                ofSetHexColor(shape->stroke.color);
                
                ofDrawBezier(p[0],p[1], p[2],p[3], p[4],p[5], p[6],p[7]);
                // end points
                ofSetColor(ofColor::cyan);
                ofDrawCircle(p[0], p[1], 2);
                ofDrawCircle(p[6], p[7], 2);
                
                // control points
                ofSetColor(ofColor::magenta);
                ofDrawCircle(p[2], p[3], 4);
                ofDrawCircle(p[4], p[5], 4);
            
                
            }
        }
    }
    
    nsvgDelete(image);*/
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){
    if(key==OF_KEY_RIGHT) {
        currentSvgIndex++;
        if(currentSvgIndex>=files.size()) currentSvgIndex = 0;
    } else if(key==OF_KEY_LEFT) {
        currentSvgIndex--;
        if(currentSvgIndex<0) currentSvgIndex = files.size()-1;
    }
}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
