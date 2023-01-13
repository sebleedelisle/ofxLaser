//
//  ofxLaserZoneTransformQuad.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#pragma once

#include "ofxLaserZoneTransformBase.h"
#include "ofxLaserDragHandle.h"
#include "ofxLaserPoint.h"
#include "ofxLaserFactory.h"
#include "ofxLaserBezierNode.h"

namespace ofxLaser {
	


class ZoneTransformLine : public ZoneTransformBase {
	
	public :
	
    ZoneTransformLine();
	~ZoneTransformLine();
    
    virtual bool update() override;
    //virtual void draw(string label) override;
    void resetNodes();
    
	virtual void init(ofRectangle& srcRect) override;

   // virtual void setHue(int hue) override;
    //virtual bool setGrid(bool snapstate, int gridsize) override;
    
	virtual bool serialize(ofJson&json) override;
	virtual bool deserialize(ofJson&jsonGroup) override;
 
    bool hitTest(ofPoint mousePoint);
   
    void getPerimeterPoints(vector<glm::vec3>& points) override;
  
    //virtual bool setSelected(bool v) override;

    virtual Point getWarpedPoint(const Point& p) override;
    virtual ofPoint getWarpedPoint(const ofPoint& p) override;
    virtual ofPoint getUnWarpedPoint(const ofPoint& p) override;
    
    glm::vec3 getVectorNormal(glm::vec3 v1, glm::vec3 v2);
    vector<BezierNode>& getNodes();
    
    void updatePolyline(); 
    void updatePerimeter();
    void updateNodes();
    bool deleteNode(int i);
    void addNode();
    
    
    
    virtual ofPoint getCentre() override;
    
    ofParameter<float> zoneWidth;
	
	protected :
	
    
//    void initListeners();
//    void removeListeners();
//
//    void mouseMoved(ofMouseEventArgs &e);
//    bool mousePressed(ofMouseEventArgs &e);
//    void mouseDragged(ofMouseEventArgs &e);
//    void mouseReleased(ofMouseEventArgs &e);
//    void paramChanged(ofAbstractParameter& e);
//
//    void updateHandleColours();
//
	
	bool initialised = false;
	
    vector<BezierNode> nodes; // all handles for all points
    ofPolyline poly;
    ofPolyline polyPerimeter; 
    
    // used to record hover position
    ofPoint mousePos;
	
	
	
	
	
	
};
	
}
