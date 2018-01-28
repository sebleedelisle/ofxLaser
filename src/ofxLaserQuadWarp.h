//
//  QuadWarp.h
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserDragHandle.h"
#include "ofxXmlSettings.h"
#include "ofxOpenCv.h"
#include "ofxLaserPoint.h"
#include "opencv2/calib3d/calib3d.hpp"



namespace ofxLaser {
class QuadWarp {
	// TODO remove listeners and tidy up on destructor
	
	public :
	
	
	QuadWarp(string savelabel = "QuadWarp", string displaylabel = "" );
	void setSrc(const ofRectangle& rect);
	void setSrc(float x, float y, float w, float h) ;
	void setDst(const ofRectangle& rect);
	void setDst(float x, float y, float w, float h) ;
	
	void draw();
	
	void initListeners();
	
	bool mousePressed(ofMouseEventArgs &e);
	bool mouseDragged(ofMouseEventArgs &e);
	bool mouseReleased(ofMouseEventArgs &e);
	
    bool hitTest(ofPoint mousepoint);

	void startDragging(int handleIndex, ofPoint clickPos);
	
	void updateCentreHandle(); 
	
	bool loadSettings();
	void saveSettings();
	
	void setVisible(bool warpvisible);
	bool checkDirty(); 
	
	cv::Mat homography;
	cv::Mat inverseHomography;
	
	void updateHomography();
	bool isVisible() { return visible; }; 
	
	Point getWarpedPoint(const Point& p);
	Point getUnWarpedPoint(const Point& p);
	ofPoint getWarpedPoint(const ofPoint& p);
	ofPoint getUnWarpedPoint(const ofPoint& p);

	string saveLabel;
	string displayLabel;
	
	const int numHandles = 4;
	DragHandle handles[4];
	DragHandle centreHandle;
	vector<DragHandle*> allHandles;
	
	vector<ofPoint> srcPoints;
	
	float width;
	float height;
	
    ofPoint offset;
	float scale = 1;
	
    bool selected; 
    
	protected :
	bool visible; 
	bool isDirty;
	
	vector<cv::Point2f> pre, post;


	
};
}
