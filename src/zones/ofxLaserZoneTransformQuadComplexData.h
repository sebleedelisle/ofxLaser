//
//  ofxLaserZoneTransformQuadComplexData.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#pragma once

#include "ofxLaserZoneTransformBase.h"
#include "ofxOpenCv.h"
#include "ofxLaserPoint.h"
#include "opencv2/calib3d/calib3d.hpp"
#include "ofxLaserWarper.h"
#include <glm/gtx/closest_point.hpp>

#include "ofxLaserGeomUtils.h"


// the basic quad transform is four points, and can either be square or
// non-square. It needs to have its four points clockwise and can't be
// inverted.
// So if it's square, the axes remain aligned, if not then


namespace ofxLaser {
    
class ZoneTransformQuadComplexData : public ZoneTransformBase {
    
    public :
    
    ZoneTransformQuadComplexData();
    ~ZoneTransformQuadComplexData();
    
    virtual bool update() override;
    
    virtual void init() override;
    void setDefault();
    
    virtual void serialize(ofJson&json) const override;
    virtual bool deserialize(ofJson&jsonGroup) override;

    void updateSrc(const ofRectangle& rect) override;
    void resetDst(const ofRectangle& rect); 
    void resetDst(glm::vec2 topleft, glm::vec2 topright, glm::vec2 bottomleft, glm::vec2 bottomright);
        
    virtual glm::vec2 getVectorToBringWithinBoundingBox() override; 
    
    
    void setFromPoints(vector<glm::vec2*> points);
    //void drag(glm::vec2 dragoffset);
    // resets to perpendicular corners
//    void resetToSquare() ;
//    bool isAxisAligned() ;
    
    //void setDst(const ofRectangle& rect);
    
//    void setDstCorners(glm::vec2 topleft, glm::vec2 topright, glm::vec2 bottomleft, glm::vec2 bottomright);
    
    bool moveHandle(int handleindex, glm::vec2 newpos, bool lockSquare);

    void getPerimeterPoints(vector<glm::vec2>& points) override;
    vector<glm::vec2> getPerimeterPoints() override;
  
    virtual Point getWarpedPoint(const Point& p) override;
    virtual ofPoint getWarpedPoint(const ofPoint& p) override;
    virtual ofPoint getUnWarpedPoint(const ofPoint& p) override;
    
    virtual glm::vec2 getCentre() override;
    glm::vec2 getDestPointAt(int i);
    int getNumPoints();

    bool setSubdivisionLevel(int newlevel);
    int getNumSubdivisions();
    void incSubdivisionLevel();
    void decSubdivisionLevel();
    
    
    
//    ofParameter<bool>useHomography;
    
   // vector<glm::vec2*> getCornerPoints();
  
    protected :

    void paramChanged(ofAbstractParameter& e);

//    void updateConvex() ;
//    bool getIsConvex() ;
  
    //void updateQuads();
//
//    float getRight() ;
//    float getLeft() ;
//    float getTop() ;
//    float getBottom() ;
    
    // returns in order top left, top right, bottom left, bottom right
    //void resetFromCorners();
    //vector<glm::vec2> getCorners();
   
    //vector<glm::vec2*> getCornerPointsClockwise();
    //bool isCorner(int index);
      
    int getPointIndexForPosition(int x, int y);
    glm::vec2& getPointForPosition(int x, int y);

   
    
    cv::Point2f toCv(glm::vec3 p) {
        return cv::Point2f(p.x, p.y);
    }
    cv::Point2f toCv(glm::vec2 p) {
        return cv::Point2f(p.x, p.y);
    }
    glm::vec3 toOf(cv::Point2f p) {
        return glm::vec3(p.x, p.y,0);
    }

    bool isConvex;
    
    bool initialised = false;
    
    vector<glm::vec2> srcPoints;
    vector<glm::vec2> dstPoints; // all handles for all points
    
    // 1 is 3, 2 is 6, 3 12 and so on
    int subdivisionLevel = 0;

    
    Warper quadWarper;
    
};
    
}
