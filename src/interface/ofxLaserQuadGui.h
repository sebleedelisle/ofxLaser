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
    void set(const QuadGui& quadToCopy);
    void setOffsetAndScale(glm::vec2 newoffset, float newscale) {
        offset = newoffset;
        scale = newscale;
    }
    void setOffset(glm::vec2 newoffset) {
        offset = newoffset;
    }
    void setScale(float newscale) {
        scale = newscale;
    } 
    void setConstrained(const ofRectangle& rect); 
    void setColours(ofColor lineColour, ofColor handleColour, ofColor labelColour);
	virtual void draw();
    ofRectangle getRectangle() {
        return ofRectangle(handles[0].x, handles[0].y, getWidth(), getHeight());
    }
    void initListeners();
    void removeListeners();
    
 
	bool mousePressed(ofMouseEventArgs &e);
    bool mouseDragged(ofMouseEventArgs &e);
    void mouseMoved(ofMouseEventArgs &e);
    bool mouseReleased(ofMouseEventArgs &e);
    
    // hit test in screen space
    // (takes into account the offset and scale)
    bool hitTestScreen(ofPoint mousepoint);
    
    // optimised hit test for checking many laser points
    bool hitTest(const ofPoint& p);

	void startDragging(int handleIndex, glm::vec3 clickPos);
	
	void updateCentreHandle();
    void updatePoly();
    
    int getWidth();
    int getHeight();
    
	virtual void serialize(ofJson&json) const ;
	virtual bool deserialize(ofJson&jsonGroup);

	void setVisible(bool warpvisible);
	bool isVisible() { return visible; };
    void setEditable(bool _editable) {
        editable = _editable;
    }
    bool getEditable() { return editable; };
    
    bool checkDirty();
    
	//string saveLabel;
	string displayLabel;
	
    bool lockPerpendicular = false;
    bool constrained = false;
    ofRectangle constrainRect;
   // bool reversable = true;
    float lineWidth = 1;
    
	const int numHandles = 4;
	DragHandle handles[4];
	DragHandle centreHandle;
	vector<DragHandle*> allHandles;
    ofPolyline quadPoly;
    ofRectangle boundingBox; 
    
	float width;
	float height;
	
    ofPoint mousePos;
    bool selected; 
    
	protected :
    
    ofPoint offset;
    float scale = 1;
 
    bool visible = true;
    bool editable = true;
    bool isDirty = true;
    bool initialised = false;
    
    
    
    private :
    ofColor lineColour;
    ofColor labelColour;
    ofColor handleColour;

   //   ofEventListener mousePressedListener;
  //    ofEventListener mouseDraggedListener;
  //    ofEventListener mouseMovedListener;
  //    ofEventListener mouseReleasedListener;

};
}
