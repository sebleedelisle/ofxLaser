//
//  LaserShape.h
//  PixelPyros
//
//  Created by Seb Lee-Delisle on 25/09/2013.
//
//

#pragma once
#include "ofxLaserPoint.h"
#include "ofxLaserRenderProfile.h"

namespace ofxLaser {
class Shape {

	public :

	Shape(){};
	virtual ~Shape(){
		//cout << "LASER SHAPE DESTROY!!!" << endl;
	};
    
    virtual Shape* clone() const =0;
    
	virtual void addPreviewToMesh(ofMesh& mesh) =0;
	
    vector<float>& getPointsAlongDistance(float distance, float acceleration, float speed, float speedMultiplier);

	vector<float> unitDistances;

    virtual glm::vec3 getStartPos() const;
    virtual glm::vec3 getEndPos() const;
	
    virtual ofFloatColor getColour() const;
    virtual ofFloatColor getColourAtPoint(int i) const;
    virtual ofFloatColor getColourAtFloatIndex(float i);
    virtual ofFloatColor getColourAtDistance(float distance);
    
    virtual bool isFilled() const;
    virtual void setFilled(bool fillstate);
    virtual void setClosed(bool closestate);
    virtual bool isClosed(); 


    glm::vec3 getPointAtDistance(float distance);
    glm::vec3 getPointAtFloatIndex(float floatIndex);
    float getFloatIndexAtDistance(float distance);
    float getLengthAtIndex(int index);
    float getLength();
    
    float getAngleAtIndexDegrees(int index); 
    
    vector<glm::vec3>& getPoints() {
        return points;
    }
    int getNumPoints() {
        return points.size();
    }
    
    void setPoints(vector<glm::vec3>& newpoints) {
        points = newpoints;
        setDirty();
    }
    void setColours(vector<ofFloatColor>& newcolours) {
        colours = newcolours ;
    }
    bool getReversable() {
        return reversable && (!closed);
    }
    
    glm::vec3 getPointAt(int i ) {
        if((i<0) || (i>=points.size())) {
            ofLogError("Shape :: getPointAt - index out of range ") << i;
            return glm::vec3(0,0,0);
        } else {
            return points[i];
        }
    }
    void update() {
        setDirty();
    }
    bool isEmpty() const {
        return points.size()==0;
    }
    bool isMultiColoured() {
        return colours.size()>1;
    }
    void clear() {
        points.clear();
        colours.clear();
        setDirty();
    }
    void addPoint(glm::vec3& p, ofFloatColor& c) {
        points.push_back(p);
        colours.push_back(c);
        setDirty();
    }
    void addPoint(glm::vec3& p) {
        points.push_back(p);
        setDirty();
    }

    float getMedianZDepth() const;
    
    virtual void setPoints(const ofPolyline& poly);
    virtual void setPoints(const vector<glm::vec3>& newpoints, bool closed);
    virtual void setColours(const vector<ofColor>& newcolours, float brightness = 1);
    virtual void setColours(const vector<ofFloatColor>& newcolours, float brightness = 1);
    virtual void setColour(const ofFloatColor colour, float brightness = 1);
    
    void setDirty(); 
   
    bool updateBoundingBox();
    bool updateLengths(); 
    
    virtual void appendPointsToVector(vector<ofxLaser::Point>& pointsToAppendTo, const RenderProfile& profile, float speedMultiplier) ;
    bool intersectsRect(ofRectangle & rect);
    
    virtual bool clipNearPlane(float nearPlaneZ) = 0;
    virtual bool clipToRectangle(ofRectangle& rect);
    vector<glm::vec3> getLineRectangleIntersectionPoints(ofRectangle& rect, glm::vec3 p1, glm::vec3 p2);
    
    bool pointInsideRect(glm::vec3& p, ofRectangle& rect); 

    // for shape sorting
    bool tested = false;
    bool reversed = false;
   
	string profileLabel = "";
	
	protected :
    
    vector <glm::vec3> points;
    vector <ofFloatColor> colours;
    bool closed = false;
    ofRectangle boundingBox;
    vector<float> lengths;
    bool boundingBoxDirty = true;
    bool lengthsDirty = true;
    float totalLength = 0; 
    bool filled = false;
    bool fillable = true;
    bool reversable = false;


};
}
