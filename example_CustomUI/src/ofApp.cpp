#include "ofApp.h"



//--------------------------------------------------------------
void ofApp::setup(){
    
    // NOTE that you no longer need to set up anything at all in ofxLaser!
    // If you want to change the size of the laser area, use
    // laser.setCanvasSize(int width, int height) - default is 800 x 800.
    
    boolParam.set("Boolean param", true);
    
    intParam.set("Integer param", 0,0,20);
    resettableIntParam.set("Integer with reset", 0,0,20);
    circleSize.set("Circle size", 100,1,400);
    circleColour.set("Circle colour", ofColor::white);
        
}

//--------------------------------------------------------------
void ofApp::update(){
    // prepares laser manager to receive new graphics
    laser.update();
}


void ofApp::draw() {
    
    ofBackground(15,15,20);

    laser.drawCircle(400, 400, circleSize, circleColour, OFXLASER_PROFILE_FAST);
    
    // sends points to the DAC
    laser.send();
  
    // PLEASE NOTE, if you just want a few custom UI elements, it's best
    // to add parameters to the Laser UI window using laser.addCustomParameter
    //
    // BUT if you want to make your own complex UI with separate windows, this
    // is how you do it :
    
    ofxLaser::UI::updateGui();
    ofxLaser::UI::startGui();
    laser.drawPreviews();
    laser.drawLaserGui();
    
    // ofxLaser::UI has a bunch of helper functions in for managing
    // ImGui functions, but you can also call methods on ImGui manually
    
    ofxLaser::UI::startWindow("Extra Window", ImVec2(660,550), ImVec2(500,350));
    
    // use ofxLaser::UI::addParameter to intelligently work out the
    // correct UI element for a particular type
    ImGui::Text("These parameters don't do anything but are merely for demonstration");
    ofxLaser::UI::addParameter(boolParam);
    ofxLaser::UI::addParameter(intParam);
    
    // or call specific functions per type which can give you more
    // control (second parameter is the default setting)
    ofxLaser::UI::addResettableIntSlider(resettableIntParam, 0);
    
    ImGui::Separator();
    ImGui::Text("These parameters control the circle");
    // wrap  in largeItemStart and largeItemEnd for bigger elements
    ofxLaser::UI::largeItemStart();

    ofxLaser::UI::addResettableFloatSlider(circleSize, 100);
    // add a '?' tool tip to the last element
    ofxLaser::UI::toolTip("Adjust the circle size");
    
    // colour UI elements have a nice colour wheel and stuff
    ofxLaser::UI::addParameter(circleColour);

    // making buttons is so easy, when the button is pressed
    // the function returns true and the code inside the
    // brackets is executed.
    if(ImGui::Button("Random Colour")) {
        circleColour = ofColor::fromHsb(ofRandom(255),255,255);
    }
    
    ofxLaser::UI::largeItemEnd();
   
     
    // call the ImGui::TreeNode method for nesting trees
    bool treevisible = ImGui::TreeNode("Foldable tree");
    if (treevisible) {
        ImGui::Text("Here is some text inside a foldable tree");
        ofxLaser::UI::addParameter(boolParam);
        ofxLaser::UI::addParameter(boolParam);
        ofxLaser::UI::addParameter(boolParam);
        
        ImGui::TreePop();
        
    }
        
    // There are so so many features of ImGui that it is
    // impossible to go into here... but have a look at how
    // I've achieved various buttons and UI elements in the
    // laser.drawLaserGui method.

    ofxLaser::UI::endWindow();
    
    ofxLaser::UI::render();
 
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(ofKeyEventArgs& e){
    
    if(e.key==OF_KEY_TAB) {
        laser.selectNextLaser();
    }
    
}
