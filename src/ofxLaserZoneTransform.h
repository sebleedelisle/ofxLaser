//
//  ofxLaserZoneTransform.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#pragma once


#include "ofxLaserDragHandle.h"
#include "ofxOpenCv.h"
#include "ofxLaserPoint.h"
#include "opencv2/calib3d/calib3d.hpp"
#include "ofxGui.h"
#include "ofxLaserWarper.h"

namespace ofxLaser {
	
class ZoneTransform {
	
	public :
	
	ZoneTransform(string labelname, string filename);
	~ZoneTransform();
	
	void initListeners();
	void removeListeners();
	
	bool mousePressed(ofMouseEventArgs &e);
	bool mouseDragged(ofMouseEventArgs &e);
	bool mouseReleased(ofMouseEventArgs &e);

	//void setName(string labelname, string filename);
	
	void setDivisions(int xdivisions, int ydivisions);
	void updateDivisions();
	void updateHomography() {
		updateQuads();
	}
	void updateQuads();
	
	void divisionsChanged(int& e);
	
	void setSrc(const ofRectangle& rect);
	//void setSrc(float x, float y, float w, float h);
	void setDst(const ofRectangle& rect);
	
	void setDstCorners(ofPoint topleft, ofPoint topright, ofPoint bottomleft, ofPoint bottomright);
	
	void update();
	void draw();
	
	bool hitTest(ofPoint mousePoint);
	
	void resetFromCorners();
	vector<ofPoint> getCorners();
	bool isCorner(int index);
	
	bool loadSettings();
	void saveSettings();
	
	void setVisible(bool warpvisible);
	bool checkDirty();

	cv::Point2f toCv(ofPoint p) {
		return cv::Point2f(p.x, p.y);
	}
	ofPoint toOf(cv::Point2f p) {
		return ofPoint(p.x, p.y);
	}
	
	Point getWarpedPoint(const Point& p);
	Point getUnWarpedPoint(const Point& p);
	ofPoint getWarpedPoint(const ofPoint& p);
	ofPoint getUnWarpedPoint(const ofPoint& p);
	
	ofParameterGroup params;
	
	ofParameter<bool>simpleMode;
	ofParameter<int>xDivisionsNew;
	ofParameter<int>yDivisionsNew;

	
	ofRectangle srcRect;
	vector<ofPoint> srcPoints;
	vector<DragHandle> dstHandles; // all handles for all points
	vector<Warper> quadWarpers;
	
	DragHandle moveHandle;
	
	string saveLabel;
	string displayLabel;
	
	
	ofPoint offset;
	float scale = 1;
	
	
	protected :
	
	bool selected;
	bool visible;
	bool isDirty;
	
	bool initialised = false;
	int xDivisions;
	int yDivisions;
	

	void drawDashedLine(ofPoint p1, ofPoint p2) {
		
		glEnable (GL_LINE_STIPPLE);
		
		glLineStipple (2, 0xAAAA);  /*  first parameter set the dash length in px  */
		glBegin(GL_LINES);
		glVertex2f (p1.x, p1.y);
		glVertex2f (p2.x, p2.y);
		glEnd();
		glDisable (GL_LINE_STIPPLE);
		glFlush ();
		
	}
	
	
	
	
	
	
	
	
	
	
};
	
	
	
	
}
