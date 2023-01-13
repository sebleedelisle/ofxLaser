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
    
    virtual bool update(){};
    //virtual void draw(string label){};
    
         
    
    // i think keep this to initialise the source rectangle
    virtual void init(ofRectangle& srcRect){};
    virtual void updateSrc(const ofRectangle& rect) ;

    //virtual void setHue(int hue);
   

    virtual void getPerimeterPoints(vector<glm::vec3>& points) = 0;


    
    //virtual bool setGrid(bool snapstate, int gridsize);
	
   // virtual bool getSelected();
   // virtual bool setSelected(bool v);
    
	
	virtual bool serialize(ofJson&json) = 0;
	virtual bool deserialize(ofJson&jsonGroup) = 0;

	//void setEditable(bool warpvisible);
    //void setVisible(bool warpvisible);
	
    void setDirty(bool state);

    virtual Point getWarpedPoint(const Point& p) = 0;
    virtual ofPoint getWarpedPoint(const ofPoint& p) = 0;
    virtual ofPoint getUnWarpedPoint(const ofPoint& p) = 0;
	
    virtual ofPoint getCentre() = 0;
	
    ofParameterGroup transformParams; 

    ofParameter<bool>locked;

	ofRectangle srcRect;
   
	
	protected :
	

	bool isDirty;

    bool isConvex;
	
	bool initialised = false;
	
	//static ofMesh dashedLineMesh;
	
	//TODO move to utils
	//static void drawDashedLine(glm::vec3 p1, glm::vec3 p2) ;
	
	
	
	
	
	
	
	
	
	
};
	
}
