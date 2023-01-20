//
//  ofxLaserZoneTransformQuadData.h
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

namespace ofxLaser {
    
class ZoneTransformQuadData : public ZoneTransformBase {
    
    public :
    
    ZoneTransformQuadData();
    ~ZoneTransformQuadData();
    
    virtual bool update() override;
    //virtual void draw(string label) override;
    
    virtual void init(ofRectangle& srcRect) override;

    //virtual void setHue(int hue) override;
    //virtual bool setGrid(bool snapstate, int gridsize) override;
    
    virtual bool serialize(ofJson&json) override;
    virtual bool deserialize(ofJson&jsonGroup) override;

    void updateSrc(const ofRectangle& rect) override;
 
    // resets to perpendicular corners
    void resetToSquare() ;
    bool isSquare() ;
    
    void setDst(const ofRectangle& rect);
    
    void setDstCorners(glm::vec3 topleft, glm::vec3 topright, glm::vec3 bottomleft, glm::vec3 bottomright);

   
    void getPerimeterPoints(vector<glm::vec2>& points) override;
  
    virtual Point getWarpedPoint(const Point& p) override;
   // virtual Point getUnWarpedPoint(const Point& p) override;
    virtual ofPoint getWarpedPoint(const ofPoint& p) override;
    virtual ofPoint getUnWarpedPoint(const ofPoint& p) override;
    
    virtual glm::vec2 getCentre() override;

    ofParameter<bool>editSubdivisions;
    ofParameter<bool>useHomography;
    ofParameter<int>xDivisionsNew;
    ofParameter<int>yDivisionsNew;

    vector<glm::vec2*> getCornerPoints();
  
    
    protected :
    
    
 //   void initListeners();
 //   void removeListeners();
    
//    void mouseMoved(ofMouseEventArgs &e);
//    bool mousePressed(ofMouseEventArgs &e);
//    void mouseDragged(ofMouseEventArgs &e);
//    void mouseReleased(ofMouseEventArgs &e);
    void paramChanged(ofAbstractParameter& e);

    
    //void updateHandleColours();
    


    void updateConvex() ;
    bool getIsConvex() ;
  
    void setDivisions(int xdivisions, int ydivisions);
    void updateDivisions();

    void updateQuads();
    
   
    void divisionsChanged(int& e);
    
    //void setSrc(float x, float y, float w, float h);
    
    
    float getRight() ;
    float getLeft() ;
    float getTop() ;
    float getBottom() ;
    
    // returns in order top left, top right, bottom left, bottom right
    void resetFromCorners();
    vector<ofPoint> getCorners();
   
//    vector<glm::vec2*> getCornerPoints();
    vector<glm::vec2*> getCornerPointsClockwise();
    bool isCorner(int index);
    
    cv::Point2f toCv(glm::vec3 p) {
        return cv::Point2f(p.x, p.y);
    }
    cv::Point2f toCv(glm::vec2 p) {
        return cv::Point2f(p.x, p.y);
    }
    glm::vec3 toOf(cv::Point2f p) {
        return glm::vec3(p.x, p.y,0);
    }
    
//    bool selected; // highlighted and ready to edit
//    bool editable;  // visible and editable
//    bool visible;
//    bool isDirty;

    bool isConvex;
    
    bool initialised = false;
    int xDivisions;
    int yDivisions;
    
    
    vector<glm::vec2> srcPoints;
    vector<glm::vec2> dstPoints; // all handles for all points
    vector<Warper> quadWarpers;
    
    // used to record hover position
    ofPoint mousePos;
    
    
    
    
    
    
};
    
}
