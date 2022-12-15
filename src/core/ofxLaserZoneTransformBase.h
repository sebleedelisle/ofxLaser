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
    virtual void draw(string label){};
    
         
    
    // i think keep this to initialise the source rectangle
    virtual void init(ofRectangle& srcRect){};
    virtual void setSrc(const ofRectangle& rect) ;

    virtual void setHue(int hue);
   
    // resets to perpendicular corners
//    virtual void resetToSquare() = 0;
//    virtual bool isSquare() = 0;

    virtual void getPerimeterPoints(vector<glm::vec3>& points) = 0;

    // scale and offset are only for the visual interface
    void setScale(float _scale) ;
    void setOffset(ofPoint _offset);
    
    virtual bool setGrid(bool snapstate, int gridsize);
	
    virtual bool getSelected();
    virtual bool setSelected(bool v);
    
    
//    virtual float getRight() = 0;
//    virtual float getLeft() = 0 ;
//    virtual float getTop() =0;
//    virtual float getBottom() = 0;
    
    //virtual bool hitTest(ofPoint mousePoint)  = 0;
    
//    virtual void updateConvex() = 0;
//    virtual bool getIsConvex() = 0;
//
    // returns in order top left, top right, bottom left, bottom right
	
	virtual bool serialize(ofJson&json) = 0;
	virtual bool deserialize(ofJson&jsonGroup) = 0;

	void setEditable(bool warpvisible);
    void setVisible(bool warpvisible);
	
    void setDirty(bool state);

    virtual Point getWarpedPoint(const Point& p) = 0;
//    virtual Point getUnWarpedPoint(const Point& p) = 0;
    virtual ofPoint getWarpedPoint(const ofPoint& p) = 0;
    virtual ofPoint getUnWarpedPoint(const ofPoint& p) = 0;
	
    virtual ofPoint getCentre() = 0;
	
    ofParameterGroup transformParams; 

    ofParameter<bool>locked;
	

    ofColor uiZoneFillColour;
    ofColor uiZoneFillColourSelected;
    ofColor uiZoneStrokeColour;
    ofColor uiZoneStrokeColourSelected;
    ofColor uiZoneStrokeSubdivisionColour;
    ofColor uiZoneStrokeSubdivisionColourSelected;
    ofColor uiZoneHandleColour;
    ofColor uiZoneHandleColourOver;
    
    
	ofRectangle srcRect;
	
    ofPoint mousePos; 
	
	
	protected :
	
	bool selected; // highlighted and ready to edit
	bool editable;  // visible and editable
    bool visible;
	bool isDirty;
    
    bool snapToGrid;
    int gridSize;
    

    bool isConvex;
	
	bool initialised = false;
	int xDivisions;
	int yDivisions;
	
    ofPoint offset;
    float scale = 1;
	//static ofMesh dashedLineMesh;
	
	//TODO move to utils
	//static void drawDashedLine(glm::vec3 p1, glm::vec3 p2) ;
	
	
	
	
	
	
	
	
	
	
};
	
}
