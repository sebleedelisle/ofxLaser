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

	void set(float xpos, float ypos, float r =8);
	void setSize(float r) ;

	void set(glm::vec3 pos);
	void set(glm::vec2 pos);
	void draw(bool isOver = true, float scale = 1) ;

	void startDrag(glm::vec3 clickPos, bool dragXAxis = true, bool dragYAxis = true, bool dontMoveWhenAltPressed = false) ;

	bool updateDrag(glm::vec3 pos);

	bool stopDrag();

	bool hitTest(glm::vec3 hitpoint) ;
	
	ofPoint clickOffset, startPos;
	bool isDragging = false;
	bool active = true;
	float radius = 4;
	bool xAxis;
	bool yAxis;
	
	bool xLocked = false;
	bool yLocked = false;
	
	bool altKeyDisable;
	bool isCircular = true;
    bool isFilled = false;
		
	bool snapToGrid = false;
	float gridSize = 1;
		
	vector<DragHandle*>connectedHandlesX;
	vector<DragHandle*>connectedHandlesY;
		
	
	ofColor col = ofColor(60);
	ofColor overCol = ofColor(255);
	
	
};
}
