//
//  QuadGui.h
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserDragHandle.h"
#include "ofxXmlSettings.h"
#include "ofxLaserPoint.h"

namespace ofxLaser {
class QuadGui {
	
	public :
	
    QuadGui();
    ~QuadGui();
    
    void setName(string savelabel = "QuadGui", string displaylabel = "");
	void set(const ofRectangle& rect);
	void set(float x, float y, float w, float h) ;
	
	void draw();
	
    void initListeners();
    void removeListeners();
    
	bool mousePressed(ofMouseEventArgs &e);
	bool mouseDragged(ofMouseEventArgs &e);
	bool mouseReleased(ofMouseEventArgs &e);
    
    void drawDashedLine(ofPoint p1, ofPoint p2); 
	
    bool hitTest(ofPoint mousepoint);

	void startDragging(int handleIndex, glm::vec3 clickPos);
	
	void updateCentreHandle(); 
	
	bool loadSettings();
	void saveSettings();
	
	void setVisible(bool warpvisible);
	bool checkDirty(); 
	
	bool isVisible() { return visible; };
	
	string saveLabel;
	string displayLabel;
	
	const int numHandles = 4;
	DragHandle handles[4];
	DragHandle centreHandle;
	vector<DragHandle*> allHandles;
	//vector<ofPoint> points;
	
	float width;
	float height;
	
    ofPoint offset;
	float scale = 1;
	
    bool selected; 
    
    
	protected :
	bool visible; 
    bool isDirty;
    
    bool initialised = false; 
	


	
};
}
