//
//  QuadWarp.h
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//

#pragma once
#include "ofxLaserDragHandle.h"
#include "ofxXmlSettings.h"

namespace ofxLaser {
class QuadWarp {
	
	
	public :
	
	QuadWarp(string savelabel = "QuadWarp");
	void set(float x, float y, float w, float h) ;
	
	void draw();
	
	void initListeners();
	
	bool mousePressed(ofMouseEventArgs &e);
	bool mouseDragged(ofMouseEventArgs &e);
	bool mouseReleased(ofMouseEventArgs &e);

	void startDragging(int handleIndex, ofPoint clickPos);
	
	bool loadSettings();
	void saveSettings();
	
	void setVisible(bool warpvisible);

	string saveLabel;
	
	const int numHandles = 4;
	DragHandle handles[4];
	DragHandle centreHandle;
	vector<DragHandle*> allHandles;
	float width;
	float height;
	
	protected :
	bool visible; 
	
};
}
