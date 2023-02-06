//
//  ofxLaserGeomUtils.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 05/02/2023.
//
//

#pragma once

#include "ofMain.h"
#include <glm/gtx/closest_point.hpp>

namespace ofxLaser {
class GeomUtils {
    
    public :
    
    static bool lineIntersectsLine(const glm::vec2 p1, const glm::vec2 p2, const glm::vec2 p3, const glm::vec2 p4) { // , glm::vec2* target = nullptr) {
        //
        //        bool targetprovided = target!=nullptr;
        //        if(!targetprovided) target = new glm::vec2(0,0);
        //        bool intersects = ofLineSegmentIntersection(p1, p2, p3, p4, *target);
        //        if(!targetprovided) delete target;
        //
        //        int areIntersecting(
        //            float v1x1, float v1y1, float v1x2, float v1y2,
        //            float v2x1, float v2y1, float v2x2, float v2y2
        //        ) {
        float d1, d2;
        float a1, a2, b1, b2, c1, c2;
        
        // Convert vector 1 to a line (line 1) of infinite length.
        // We want the line in linear equation standard form: A*x + B*y + C = 0
        // See: http://en.wikipedia.org/wiki/Linear_equation
        //a1 = v1y2 - v1y1;
        a1 = p2.y - p1.y;
        //b1 = v1x1 - v1x2;
        b1 = p1.x - p2.x;
        //c1 = (v1x2 * v1y1) - (v1x1 * v1y2);
        c1 = (p2.x * p1.y) - (p1.x * p2.y);
        
        // Every point (x,y), that solves the equation above, is on the line,
        // every point that does not solve it, is not. The equation will have a
        // positive result if it is on one side of the line and a negative one
        // if is on the other side of it. We insert (x1,y1) and (x2,y2) of vector
        // 2 into the equation above.
        d1 = (a1 * p3.x) + (b1 * p3.y) + c1;
        d2 = (a1 * p4.x) + (b1 * p4.y) + c1;
        
        // If d1 and d2 both have the same sign, they are both on the same side
        // of our line 1 and in that case no intersection is possible. Careful,
        // 0 is a special case, that's why we don't test ">=" and "<=",
        // but "<" and ">".
        if (d1 > 0 && d2 > 0) return false;
        if (d1 < 0 && d2 < 0) return false;
        
        // The fact that vector 2 intersected the infinite line 1 above doesn't
        // mean it also intersects the vector 1. Vector 1 is only a subset of that
        // infinite line 1, so it may have intersected that line before the vector
        // started or after it ended. To know for sure, we have to repeat the
        // the same test the other way round. We start by calculating the
        // infinite line 2 in linear equation standard form.
        a2 = p4.y - p3.y;
        b2 = p3.x - p4.x;
        c2 = (p4.x * p3.y) - (p3.x * p4.y);
        
        // Calculate d1 and d2 again, this time using points of vector 1.
        d1 = (a2 * p1.x) + (b2 * p1.y) + c2;
        d2 = (a2 * p2.x) + (b2 * p2.y) + c2;
        
        // Again, if both have the same sign (and neither one is 0),
        // no intersection is possible.
        if (d1 > 0 && d2 > 0) return false;
        if (d1 < 0 && d2 < 0) return false;
        
        // If we get here, only two possibilities are left. Either the two
        // vectors intersect in exactly one point or they are collinear, which
        // means they intersect in any number of points from zero to infinite.
        if ((a1 * b2) - (a2 * b1) == 0.0f) return false;
        
        // If they are not collinear, they must intersect in exactly one point.
        return true;
    }
    

    
    static bool pointInPoly(const glm::vec2 p, const vector<glm::vec2>& polypoints, float boundleft) {
        if(polypoints.size()<3) return false;
        
        int intersectcount = 0;
        
        glm::vec2 raystart = polypoints[0] - glm::vec2(1,0);
        
        for(int i = 0; i<polypoints.size(); i++) {
            const glm::vec2& p1 =  polypoints[i];
            const glm::vec2& p2 = polypoints[(i+1)%polypoints.size()];
            if(lineIntersectsLine(p1, p2, raystart, p)) {
                intersectcount++;
            }
            
        }
        return (intersectcount%2)==1;
            
        
    
    }
    static bool pointInPoly(const glm::vec2 p1, const vector<glm::vec2>& polypoints) {
        if(polypoints.size()<3) return false;
        float boundleft = polypoints[0].x;
        for(const glm::vec2& p : polypoints) {
            if(boundleft>p.x) boundleft = p.x;
        }
        return pointInPoly (p1, polypoints, boundleft);
        
    }

    static bool clampToVector(glm::vec2& pointToClamp, const glm::vec2& p1, const glm::vec2&p2){
 
        glm::vec2 clamped = getClampedToVector(pointToClamp, p1, p2, true, false);
        if(clamped!=pointToClamp) {
            pointToClamp = clamped;
            return true;
        } else {
            return false;
        }
    }

    static glm::vec2 getClampedToVector(const glm::vec2& source, const glm::vec2& p1, const glm::vec2&p2, bool clampinside, bool clampoutside){
        
        //glm::vec2 returnvector;
        
        glm::vec2 v1 = p2-p1;
        v1 = glm::rotate(v1, ofDegToRad(-90));
        glm::vec2 v2 = source - p1;
        
        float d = glm::dot(v2, v1);
        
        // if it's too short then we don't have a valid vector to project onto
        if(glm::length(v1)<0.01) {
            ofLogNotice() << v1 << " " << v2;
            return p1;
        }
        
        if(((d<0) && (clampinside) ) || ((d>0) && (clampoutside))) {
            glm::vec2 normal = glm::normalize(v1);
            d = glm::dot(v2, normal);
            
            return source-(normal*d);

        } else {
            return source;
        }
        
    }

    static bool isWindingClockwise(const vector<glm::vec2*>& corners){
        
        //vector<glm::vec2*> corners = getCornerPointsClockwise();
        float sum = 0;
        for(int i = 0; i<4; i++) {
            const glm::vec2& p1 = *corners[(i+1)%4];
            const glm::vec2& p2 = *corners[i];
            glm::vec2 edge(p2.x-p1.x, p2.y+p2.y);
            sum+=(edge.x*edge.y);
        }
        return sum>=0;
        
    }
    
    static bool isConvex(const vector<glm::vec2*>& points){
        bool convex = true;
        for(size_t i = 0; i<4; i++) {
        
            ofVec2f p1 = *points[i%4];
            ofVec2f p2 = *points[(i+1)%4];
            ofVec2f p3 = *points[(i+2)%4];
            ofVec2f v1 = p2-p1;
            ofVec2f v2 = p3-p2;
            v2.rotate(90);
            if(v2.dot(v1)>0) convex = false;
        }
        
        if(convex) {
            for(int i =0; i<3; i++) {
                for(int j = i+1; j<4; j++) {
                    if(points[i] == points[j]) {
                        convex = false;
                    }
                }
            }
        }
        return convex;
    }
        
    
    static float pointDistanceFromLine(const glm::vec2 pos, const glm::vec2 p1, const glm::vec2 p2) {
        
        glm::vec2 closestpoint = glm::closestPointOnLine(pos, p1, p2);
        
        return glm::distance(closestpoint, pos);

    }
    
    
    static float getMinimumCrossSectionWidth(const vector<glm::vec2>& points) {
        
        float closestDistance = INFINITY;
        for(int i=0; i<points.size(); i++) {
            // get one edge at a time
            const glm::vec2& p1 = points[i];
            const glm::vec2& p2 = points[(i+1)%points.size()];
            
            float maxDistance = 0;
            for(const glm::vec2 p : points) {
                float distance = glm::distance(p, getClampedToVector(p, p1, p2, true, true));
                if(distance>maxDistance) maxDistance = distance;
            }
            
            if(maxDistance<closestDistance) {
                closestDistance = maxDistance;
            }
        }
        return closestDistance;
            
    }

};
}
