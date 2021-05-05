//
//  ofxLaserZoneTransform.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#pragma once


#include "ofxLaserDragHandle.h"
#include "ofxOpenCv.h"
#include "ofxLaserPoint.h"
#include "opencv2/calib3d/calib3d.hpp"
#include "ofxLaserUI.h"
#include "ofxLaserWarper.h"

namespace ofxLaser {
	
class ZoneTransform {
	
	public :
	
	ZoneTransform();
	~ZoneTransform();
    
    bool update();
    void draw(string label);
    
	void init(ofRectangle& srcRect);

	
	void initListeners();
	void removeListeners();
	
    bool mouseMoved(ofMouseEventArgs &e);
    bool mousePressed(ofMouseEventArgs &e);
    bool mouseDragged(ofMouseEventArgs &e);
	bool mouseReleased(ofMouseEventArgs &e);
    void paramChanged(ofAbstractParameter& e);

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

	
	void setHandleSize(float size);
	
    bool getSelected() {return selected;};
    float getRight() {
        float right = 0;
        for(DragHandle& handle : dstHandles) {
            if(handle.x>right) right = handle.x;
        }
        right*=scale; 
        right+=offset.x;
        
        return right;
    }
    
	bool hitTest(ofPoint mousePoint);
	
	void resetFromCorners();
	vector<ofPoint> getCorners();
	bool isCorner(int index);
	
//    string getSaveLabel() {
//        return "Projector"+ofToString(projectorIndex)+"Zone"+ofToString(zoneIndex);
//
//    }
    
	//bool loadSettings();
	//void saveSettings();
	void serialize(ofJson&json);
	bool deserialize(ofJson&jsonGroup);

	void setEditable(bool warpvisible);
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
	
	//string saveLabel;
	//string displayLabel;
	//int zoneIndex;
    //int projectorIndex;
	
	
	ofPoint offset;
	float scale = 1;
    ofPoint mousePos; 
	
	
	protected :
	
	bool selected; // highlighted and ready to edit
	bool editable;  // visible and editable
    bool visible;
	bool isDirty;
	
	bool initialised = false;
	int xDivisions;
	int yDivisions;
	
	//static ofMesh dashedLineMesh;
	
	//TODO move to utils
	//static void drawDashedLine(glm::vec3 p1, glm::vec3 p2) ;
	
	
	
	
	
	
	
	
	
	
};
	
}
