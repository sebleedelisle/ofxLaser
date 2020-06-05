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
	
	ZoneTransform(int index, string filename);
	~ZoneTransform();
	
	void init(ofRectangle& srcRect);
	void initGuiListeners();
	
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
	
	void setDstCorners(glm::vec3 topleft, glm::vec3 topright, glm::vec3 bottomleft, glm::vec3 bottomright);
	
	void update();
	void draw();
	
	bool hitTest(ofPoint mousePoint);
	
	void resetFromCorners();
	vector<ofPoint> getCorners();
	bool isCorner(int index);
	
	bool loadSettings();
	void saveSettings();
	void serialize(ofJson&json);
	bool deserialize(ofJson&jsonGroup);

	
	void setVisible(bool warpvisible);
	bool checkDirty();
	void setDirty(bool state) {isDirty = state;};

	cv::Point2f toCv(glm::vec3 p) {
		return cv::Point2f(p.x, p.y);
	}
	glm::vec3 toOf(cv::Point2f p) {
		return glm::vec3(p.x, p.y,0);
	}
	
	Point getWarpedPoint(const Point& p);
	Point getUnWarpedPoint(const Point& p);
	ofPoint getWarpedPoint(const ofPoint& p);
	ofPoint getUnWarpedPoint(const ofPoint& p);
	
	ofPoint getCentre(); 
	
	ofParameterGroup params;
	
	ofParameter<bool>editSubdivisions;
	ofParameter<bool>useHomography;
	ofParameter<int>xDivisionsNew;
	ofParameter<int>yDivisionsNew;

	
	ofRectangle srcRect;
	vector<glm::vec3> srcPoints;
	vector<DragHandle> dstHandles; // all handles for all points
	vector<Warper> quadWarpers;
	
	DragHandle moveHandle;
	
	string saveLabel;
	string displayLabel;
	int index; 
	
	
	ofPoint offset;
	float scale = 1;
	
	
	protected :
	
	bool selected;
	bool visible;
	bool isDirty;
	
	bool initialised = false;
	int xDivisions;
	int yDivisions;
	
	static ofMesh dashedLineMesh;
	
	//TODO move to utils
	static void drawDashedLine(glm::vec3 p1, glm::vec3 p2) ;
	
	
	
	
	
	
	
	
	
	
};
	
}
