//
//  ofxLaserZoneTransformQuad.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#pragma once

#include "ofxLaserPoint.h"

namespace ofxLaser {
	
class ZoneTransformBase {
	
	public :
	
    ZoneTransformBase();
    ~ZoneTransformBase();
    
    virtual bool update(){return false;};

    // i think keep this to initialise the source rectangle
    virtual void init(){};
    virtual void updateSrc(const ofRectangle& rect) ;

    virtual void getPerimeterPoints(vector<glm::vec2>& points) = 0;
    virtual vector<glm::vec2> getPerimeterPoints();
    
	virtual void serialize(ofJson&json) const = 0;
	virtual bool deserialize(ofJson&jsonGroup) = 0;
	
    void setDirty(bool state);

    virtual Point getWarpedPoint(const Point& p) = 0;
    virtual ofPoint getWarpedPoint(const ofPoint& p) = 0;
    virtual ofPoint getUnWarpedPoint(const ofPoint& p) = 0;
	
    virtual glm::vec2 getCentre() = 0;
	
    ofParameterGroup transformParams; 

    ofParameter<bool>locked;

	ofRectangle srcRect;
	
	protected :

	bool isDirty;

    bool isConvex;
	
	bool initialised = false;
	
	
	
};
	
}
