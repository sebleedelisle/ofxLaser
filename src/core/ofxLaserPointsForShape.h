//
//  ofxLaserPointsForShape.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 31/08/2021.
//
#include "ofxLaserPoint.h"
namespace ofxLaser {

// a container than holds all the points for a shape
// it extends a vector of ofxLaser::Point objects
class PointsForShape : public vector<Point> {
    
    public:
    bool tested = false;
    bool reversed = false;
    bool reversable = true;
    Point& getStart() {
        return reversed?this->back() : this->front();
    }
    Point& getEnd() {
        return reversed?this->front() : this->back();
    }
    
    glm::vec3 getStartGlm() {
        glm::vec3 p;
        p.x = reversed ? this->back().x : this->front().x;
        p.y = reversed ? this->back().y : this->front().y;
        return p;
    }
    glm::vec3 getEndGlm() {
        glm::vec3 p;
        p.x = reversed ? this->front().x : this->back().x;
        p.y = reversed ? this->front().y : this->back().y;
        return p;
    }
    
};

}
