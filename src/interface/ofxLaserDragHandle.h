//
//  DragHandle.h
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//
#pragma once

#include "ofMain.h"

namespace ofxLaser {
	class DragHandle : public glm::vec3{
	
	public :
	DragHandle();
		
	DragHandle(glm::vec3 p);
	DragHandle(float x, float y);

    void set(float xpos, float ypos) {
        set(xpos, ypos, size);
    }
    void set(float xpos, float ypos, float size);
    void setSize(float size) ;

	void set(glm::vec3 pos);
	void set(glm::vec2 pos);
	void draw(const glm::vec3& mousepos, float scale = 1);
    void draw(bool isOver = true, float scale = 1);
    
    void setColour(ofColor colour, ofColor overColour) {
        col = colour;
        overCol = overColour; 
        
    }
    
    void setGrid(bool snapstate, int gridsize) {
        snapToGrid = snapstate;
        gridSize = gridsize; 
    }
        
	void startDrag(glm::vec3 clickPos, bool dragXAxis = true, bool dragYAxis = true, bool dontMoveWhenAltPressed = false) ;

	bool updateDrag(glm::vec3 pos);

	bool stopDrag();

	bool hitTest(glm::vec3 hitpoint, float scale = 1) ;
	
	ofPoint clickOffset, startPos;
	bool isDragging = false;
    bool active = true;
	float size = 10;
	bool xAxis;
	bool yAxis;
	
	bool xLocked = false;
	bool yLocked = false;
	
	bool altKeyDisable;
	bool isCircular = false;
    bool isFilled = true;
		
		
	vector<DragHandle*>connectedHandlesX;
	vector<DragHandle*>connectedHandlesY;
		
	
	ofColor col = ofColor(255, 100);
	ofColor overCol = ofColor(255,255);
	
    protected :
    bool snapToGrid = false;
    float gridSize = 1;

};
}
