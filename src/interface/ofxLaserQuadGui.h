//
//  QuadGui.h
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserDragHandle.h"
#include "ofxLaserPoint.h"
#include "ofxLaserUI.h"

namespace ofxLaser {
class QuadGui {
	
	public :
	
    QuadGui();
    ~QuadGui();
    
    void setName(string displaylabel = "");
	void set(const ofRectangle& rect);
	virtual void set(float x, float y, float w, float h) ;
    void setConstrained(const ofRectangle& rect); 
	
	void draw();
	
    void initListeners();
    void removeListeners();
    
	bool mousePressed(ofMouseEventArgs &e);
    bool mouseDragged(ofMouseEventArgs &e);
    bool mouseMoved(ofMouseEventArgs &e);
    bool mouseReleased(ofMouseEventArgs &e);
    
    bool hitTest(ofPoint mousepoint);

	void startDragging(int handleIndex, glm::vec3 clickPos);
	
	void updateCentreHandle(); 
	
	//bool loadSettings();
	//void saveSettings();
	void serialize(ofJson&json);
	bool deserialize(ofJson&jsonGroup);

	void setVisible(bool warpvisible);
	bool checkDirty(); 
	
	bool isVisible() { return visible; };
	
	//string saveLabel;
	string displayLabel;
	
    bool lockPerpendicular = false;
    bool constrained = false;
    ofRectangle constrainRect;
   // bool reversable = true;

    ofColor lineColour; 
    
	const int numHandles = 4;
	DragHandle handles[4];
	DragHandle centreHandle;
	vector<DragHandle*> allHandles;

	float width;
	float height;
	
    ofPoint offset;
	float scale = 1;
    ofPoint mousePos; 
    bool selected; 
    
	protected :
	bool visible = true;
    bool isDirty = true;
    
    bool initialised = false; 
	

};
}
