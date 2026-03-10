//
//  ofxLaserPointsForShape.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 31/08/2021.
//
#include "ofxLaserPoint.h"
#include <string>
namespace ofxLaser {

// a container than holds all the points for a shape
// it extends a vector of ofxLaser::Point objects
class PointsForShape : public vector<Point> {
    
    public:
    
    PointsForShape() = default;
    PointsForShape(const PointsForShape&) = default;
    PointsForShape& operator=(const PointsForShape&) = default;
    // Move operations are explicitly defaulted so std::move on this type
    // transfers the underlying point buffer instead of forcing deep copies.
    PointsForShape(PointsForShape&&) noexcept = default;
    PointsForShape& operator=(PointsForShape&&) noexcept = default;
    
    bool tested = false;
    bool reversed = false;
    bool reversable = true;
    // Zone UID for per-zone path ordering/memory.
    std::string zoneUid;
    Point& getStart() {
        return reversed?this->back() : this->front();
    }
    // Const overload keeps read-only callers (e.g. distance calculations)
    // on the fast path without forcing copies or const_casts.
    const Point& getStart() const {
        return reversed?this->back() : this->front();
    }
    Point& getEnd() {
        return reversed?this->front() : this->back();
    }
    const Point& getEnd() const {
        return reversed?this->front() : this->back();
    }
    
    glm::vec3 getStartGlm() {
        glm::vec3 p;
        p.x = reversed ? this->back().x : this->front().x;
        p.y = reversed ? this->back().y : this->front().y;
        return p;
    }
    glm::vec3 getStartGlm() const {
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
    glm::vec3 getEndGlm() const {
        glm::vec3 p;
        p.x = reversed ? this->front().x : this->back().x;
        p.y = reversed ? this->front().y : this->back().y;
        return p;
    }
    
};

}
