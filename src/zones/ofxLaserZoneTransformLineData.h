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
    


class ZoneTransformLineData : public ZoneTransformBase {
    
    public :
    
    ZoneTransformLineData();
    ~ZoneTransformLineData();
    
    virtual bool update() override;
    
    void resetNodes();
    
    virtual void init() override;
    
    bool setFromPoints(const vector<glm::vec2*> points);
    void updateCurves(); 
    
    bool moveHandle(int handleindex, glm::vec2 newpos);
    virtual void serialize(ofJson&json) const override;
    virtual bool deserialize(ofJson&jsonGroup) override;
 
    void getPerimeterPoints(vector<glm::vec2>& points) override;
    
  
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
    
    bool autoSmooth = true;
    float smoothLevel = 0.4;
    
    virtual glm::vec2 getCentre() override;
    
    ofParameter<float> zoneWidth;
    
    protected :

    void paramChanged(ofAbstractParameter& e);

    bool initialised = false;
    
    vector<BezierNode> nodes; // all handles for all points
    ofPolyline poly;
    ofPolyline polyPerimeter;
    
    
};
    
}
