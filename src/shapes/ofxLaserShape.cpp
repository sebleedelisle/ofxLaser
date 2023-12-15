//
//  LaserShape.h
//
//  Created by Seb Lee-Delisle on 20/05/2021.
//
//

#include "ofxLaserShape.h"

using namespace ofxLaser;

vector<float>& Shape :: getPointsAlongDistance(float distance, float acceleration, float speed, float speedMultiplier){
    
    unitDistances.clear();
    
    if(isEmpty()) return unitDistances;
    
    speed*=speedMultiplier;
    acceleration*=speedMultiplier;
    
    
    // otherwise, we'll get NaNs! 
    if(distance == 0) {
        unitDistances.push_back(0);
        return unitDistances;
        
    }
    
    float acceleratedistance = (speed*speed) / (2*acceleration);
    float timetogettospeed = speed / acceleration;
    
    float totaldistance = distance;
    
    float constantspeeddistance = totaldistance - (acceleratedistance*2);
    float constantspeedtime = constantspeeddistance/speed;
    
    if(totaldistance<(acceleratedistance*2)) {
        
        constantspeeddistance = 0 ;
        constantspeedtime = 0;
        acceleratedistance = totaldistance/2;
        speed = sqrt( acceleratedistance * 2 * acceleration);
        timetogettospeed = speed / acceleration;
        
    }
    
    float totaltime = (timetogettospeed*2) + constantspeedtime;
    
    float timeincrement = totaltime / (floor(totaltime));
    
    float currentdistance;
    
    float t = 0;
    
    while (t <= totaltime + 0.001) {
        
        if(t>totaltime) t = totaltime;
        
        if(t <=timetogettospeed) {
            currentdistance = 0.5 * acceleration * (t*t);
            
        } else if((t>timetogettospeed) && (t<=timetogettospeed+constantspeedtime)){
            currentdistance = acceleratedistance + ((t-timetogettospeed) * speed);
            
        } else  {
            float t3 = t - (timetogettospeed + constantspeedtime);
            
            currentdistance = (acceleratedistance + constantspeeddistance) + (speed*t3)+(0.5 *(-acceleration) * (t3*t3));
            
            
        }
        
        unitDistances.push_back(currentdistance/totaldistance);
        
        t+=timeincrement;
        
    }
    
    return unitDistances;
    
}


glm::vec3 Shape :: getStartPos() const{
    if(isEmpty()) {
        ofLogError("Shape::getStartPos - shape is empty");
        glm::vec3(0,0,0);
    }
    if(reversed && reversable) return points.back();
    else return points.front();
}
glm::vec3 Shape :: getEndPos() const{
    if(isEmpty()) {
        ofLogError("Shape::getEndPos - shape is empty");
        glm::vec3(0,0,0);
    }
    if(closed) return getStartPos();
    if(reversed && reversable) return points.front();
    else return points.back();
};

ofFloatColor Shape :: getColour() const{
    
    if(colours.size()>0) {
        return colours.at(0);
    } else {
        ofLogError("ofxLaser :: Shape :: getColour - no colours available! This shouldn't happen. ");
        return ofColor::white;
    }
}

 ofFloatColor Shape :: getColourAtPoint(int i) const {
     //if(isEmpty()) return ofColor::white;
     if(colours.size()==0) {
         ofLogError("Shape :: getColourAtPoint - no colours in this shape!!!");
         return ofColor ::white;
     }
     if(colours.size()==1) return colours.at(0);
     if(i<0) return colours.at(0);
     
     int numPoints = closed ? points.size()+1 : points.size();
     i = ofClamp(i, 0, numPoints-1);
     
     return colours.at(i%colours.size());
     
}

ofFloatColor Shape ::getColourAtFloatIndex(float i){
    //if(isEmpty()) return ofColor::white;
    if(colours.size()==0) {
        ofLogError("Shape :: getColourAtFloatIndex - no colours in this shape!!!");
        return ofColor ::white;
    }
    
    if(colours.size()==1) return colours[0];
    int index1 = floor(i);
    int index2 = ceil(i);
    
    if(index1 == index2) return colours.at(index1%colours.size());
    
    ofFloatColor c1 = colours.at(index1%colours.size());
    ofFloatColor c2 = colours.at(index2%colours.size());
    c1.lerp(c2, fmod(i,1));
    return c1;
}
ofFloatColor Shape ::getColourAtDistance(float distance) {
    if(isEmpty()) {
        ofLogError("Shape :: getColourAtDistance - no points in this shape!!!");
        return ofColor ::white;
    }
    float floatindex = getFloatIndexAtDistance(distance);
    if(floatindex>points.size()) {
        ofLogNotice("getColourAtDistance past end of points") << floatindex;
    }
    
    return getColourAtFloatIndex(floatindex);
    
}



void Shape :: setDirty() {
    lengthsDirty = boundingBoxDirty = true;
}

void Shape :: setPoints(const ofPolyline& poly) {
    points = poly.getVertices();
   
    closed = poly.isClosed();

    setDirty();
}
void Shape :: setPoints(const vector<glm::vec3>& newpoints, bool _closed) {
    points = newpoints;
    closed = _closed;
    setDirty();
}

void Shape :: setColours(const vector<ofColor>& newcolours, float brightness){
    colours.clear();
    for(const ofColor& c : newcolours) {
        colours.push_back(c*brightness);
    }
    
}
void Shape :: setColours(const vector<ofFloatColor>& newcolours, float brightness) {
    colours = newcolours;
    for(ofFloatColor& c : colours) {
        c*=brightness;
    }
}
void Shape :: setColour(const ofFloatColor colour, float brightness){
    colours = {colour * brightness};

}

void Shape :: setClipRectangle(const ofRectangle& rect) {
    clipRectangle = rect;
}
ofRectangle Shape :: getClipRectangle() {
    return clipRectangle;
} 
void Shape :: setIntersectionClipRectangle(ofRectangle& rect) {
    clipRectangle = clipRectangle.getIntersection(rect);
    
}


void Shape :: appendPointsToVector(vector<ofxLaser::Point>& pointsToAppendTo, const RenderProfile& profile, float speedMultiplier) {
    
};

bool Shape :: updateBoundingBox() {
    if(isEmpty()) return false;
    if(boundingBoxDirty) {
        bool first = true;
    
        for(glm::vec3& p : points) {
            if(first) {
                boundingBox.set(p, 0,0);
                first = false;
            } else {
                boundingBox.growToInclude(p);
            }
        }

        boundingBoxDirty = false; 
        return true;
    } else {
        return false;
    }
}

bool Shape :: updateLengths() {
    if(isEmpty()) return false;
    if(lengthsDirty) {
        
        lengths.clear();
        totalLength = 0;
        
        // lengths should have 1 length for every point
        // if the shape is closed then it should have one more
        
        int numLengths = closed? points.size()+1 : points.size();
        
        lengths.push_back(0); // first length always zero!
        
        for(int i = 0; i<numLengths-1; i++ ) {
            
            float l = glm::distance(points[i], points[(i+1)%points.size()]);
            lengths.push_back(l+totalLength);
            totalLength+=l;
            
        }
        
        lengthsDirty = false;
        return true;
    } else {
        return false;
    }
    
}

glm::vec3 Shape :: getPointAtDistance(float distance) {
    if(isEmpty()) {
        ofLogError("Shape :: getPointAtDistance - no points in this shape!!!");
        return glm::vec3(0,0,0);
    }
    //updateLengths();
    
    return getPointAtFloatIndex(getFloatIndexAtDistance(distance));
    

}

glm::vec3 Shape ::getPointAtFloatIndex(float floatIndex) {
    if(isEmpty()) {
        ofLogError("Shape :: getPointAtFloatIndex - no points in this shape!!!");
        return glm::vec3(0,0,0);
    }
    if(floatIndex <=0) return points.at(0);
    
    int lastPointIndex = closed? points.size() : points.size()-1;
    
    if(floatIndex>=lastPointIndex) return points.at(lastPointIndex%points.size());

    int roundedIndex = floor(floatIndex);
    glm::vec3& p1 = points.at(roundedIndex);
    glm::vec3& p2 = points.at((roundedIndex+1)%points.size());
    return glm::mix(p1, p2, fmod(floatIndex, 1));
 
}
    
float Shape :: getFloatIndexAtDistance(float distance) {
    if(isEmpty()) {
        ofLogError("Shape :: getFloatIndexAtDistance - no points in this shape!!!");
        return 0;
    }
    if(points.size()<2) return 0;
    updateLengths();
    
    if(distance>=totalLength) return lengths.size();
    else if(distance<=0) return 0;
    
//    int lastPointIndex = closed? points.size() : points.size()-1;
    
    
    int i = 0;
    // lengths should automatically contain enough lengths whether the line
    // is open or closed
    
    while(lengths.at(i)<distance && i<lengths.size()) i++;
    
//    while((segmentEndLength<distance) && (i<lengths.size())) {
//        segmentStartLength = segmentEndLength;
//        segmentEndLength=lengths[i];
//        i++;
//    }
//
    float segmentEndLength = lengths.at(i);
    float segmentStartLength = lengths.at(i-1);

    
    return i-1 + ofMap(distance, segmentStartLength, segmentEndLength, 0, 1);
    
}

float Shape :: getLengthAtIndex(int index) {
    if(isEmpty())  {
        ofLogError("Shape :: getLengthAtIndex - no points in this shape!!!");
        return 0;
    }
    updateLengths();
    
    index = ofClamp(index, 0, lengths.size()-1);
    // if closed, lengths should always have 1 more than points
    return lengths[index];
    
}

float Shape :: getLength() {
    if(isEmpty())  {
        ofLogError("Shape :: getLength - no points in this shape!!!");
        return 0;
    }
    updateLengths();
    return totalLength;
}

bool Shape :: intersectsRect(ofRectangle & rect) {
    if(isEmpty())  {
        ofLogError("Shape :: intersectsRect - no points in this shape!!!");
        return 0;
    }
    
    if(points.size()==1) {
        return pointInsideRect(points[0], rect);
    }
    
    updateBoundingBox();
    
    if(!rect.intersects(boundingBox)) return false;
    
    const vector<glm::vec3> & vertices = points;
    for(size_t i = 1; i< vertices.size(); i++) {
        if(rect.intersects(vertices[i-1],vertices[i])) return true;
    }
    return false;
    
    
}

float Shape :: getAngleAtIndexDegrees( int index) {
    
    if(isEmpty())  {
        ofLogError("Shape :: getAngleAtIndexDegrees - no points in this shape!!!");
        return 0;
    }
    
    if(index <=0) return 0;
    else if(index>=points.size()-1) return 0;
    
    glm::vec3 p1 = points.at(index-1);
    glm::vec3 p2 = points.at(index);
    glm::vec3 p3 = points.at(index+1);
   // glm::vec3 epsilon(0.001,0.001,0.001);
    float epsilon = 0.001f;
    if((glm::distance2(p1, p2)<epsilon) || glm::distance2(p2, p3)<epsilon)  return 0;
    
    glm::vec3 v1 = glm::normalize(p2-p1);
    glm::vec3 v2 = glm::normalize(p2-p3);
//    if((glm::length2(v1)==0) || (glm::length2(v2)==0)) return 0;
        
    float angle = glm::pi<float>() - acosf( ofClamp( glm::dot( v1, v2 ), -1.f, 1.f ) );
    return ofRadToDeg(angle);
    
}


bool Shape :: clipToRectangle() {
    
    ofRectangle& rect = clipRectangle;
    
    if(isEmpty()) return false;
    bool changed = false;
    
    bool anyinside = false;
    glm::vec3 lastpoint = points[0];
    for(glm::vec3& p : points) {
        if(pointInsideRect(p, rect)) {
            anyinside = true;
            break;
        }
        if(doesLineIntersectRect(rect, p, lastpoint)) {
            anyinside = true;
            break;
        }
        lastpoint = p;
        
    }
    if(!anyinside) {
        points.clear();
        colours.clear();
        setColour(ofColor::black);
        return true;
    }
    
    vector<glm::vec3> & vertices = points;
    vector<Point> segmentPoints;
    vector<vector<Point>> newsegments;
    
    bool outside = true;
    glm::vec3 previousPoint = vertices.front();
    
    // go through each point
    for(int i = 0; i<=vertices.size(); i++) {
        
        if(i==vertices.size()&&!closed) break;
        
        glm::vec3& p = vertices.at(i%vertices.size());
       
        // if it's outside the rectangle
        
        if(!pointInsideRect(p, rect)) { // if we're outside
            
            // and we're not already outside, or else this line passes through
            bool leavesRect = (!outside);
            bool intersectsRect = ((i>0)&&(rect.intersects(previousPoint, p)) &&(getColourAtPoint(i)!=ofColor::black) &&(getColourAtPoint(i-1)!=ofColor::black) );
            
          //  ofLogNotice("Intersects rect ") << intersectsRect << "  leaves rect : " << leavesRect;

            // two different cases here, either we are going from inside the rectangle
            // to outside the rectangle, or else we are going from outside the rectangle
            // to outside, but passing through.
            
            if(leavesRect || intersectsRect) { // and we weren't outside last time, or we're crossing through
                
                outside = true;
                // if we already have points then add an end point
                // for the shape that is on the edge of the rectangle
                if(i>0) {
                    
                    vector<glm::vec3> intersections = getLineRectangleIntersectionPoints(rect, previousPoint, p);
                    
                    glm::vec3 intersection = p;
                    
                    if(intersections.size()==0) {
                        // probably when a point is exactly on the edge?
                        //ofLogError("Shape :: clipToRectangle - weird, should have found an intersection for this line... ");
                    } else {
                        intersection = intersections[0];
                    }
                    // todo calculate where we are between points
                    float trimFactor = 0;
                    // trimfactor should be the unit position between the points.
                    // previousPoint to intersectionpoint divided by
                    // previousPoint to current point
                    float l1 = glm::distance(previousPoint, intersection);
                    float l2 = glm::distance(previousPoint, p);
                    if(l2>0) trimFactor = l1/l2;
                    

                    ofColor c = getColourAtFloatIndex(i-1+trimFactor);
              
                    segmentPoints.push_back(Point(intersection, c));
                    
                    if(intersections.size()>1) {
                        segmentPoints.push_back(Point(intersections.back(), c));
                    }

                    newsegments.push_back(segmentPoints);
                    
                    segmentPoints.clear();
                    
                }
                
            }
            changed = true;
            // otherwise if we are already behind then we don't need to do
            // anything except ignore this point
        
        } else { // else if we are in front of the plan...
            // and we're currently behind...
            if(outside) {
                outside = false;
                // if we are not at the first point
                if(i>0) {
                    // then get the edge position
   
                    vector<glm::vec3> intersections = getLineRectangleIntersectionPoints(rect, previousPoint, p);
                    
                    glm::vec3 intersection = p;
                    
                    if(intersections.size()==0) {
                        // probably when a point is on the edge exactly?
                        ofLogError("Shape :: clipToRectangle - weird, should have found an intersection for this line... ");
                    } else {
                        intersection = intersections.front();
                       
                    }
                    
                    float trimFactor = 1;// ofMap(nearPlaneZ, previousPoint.z, p.z, 0, 1);
                    float l1 = glm::distance(previousPoint, intersection);
                    float l2 = glm::distance(previousPoint, p);
                    if(l2>0) trimFactor = l1/l2;
                    
                    
                    ofColor c = getColourAtFloatIndex(i-1+trimFactor);
                    segmentPoints.push_back(Point(intersection, c));
                    
                    changed = true;
                }
            }
            
            // either way, if we're still inside, add this next point
            if(!outside) segmentPoints.push_back(Point(p, getColourAtPoint(i)));
                
            
        }
        previousPoint = p;
    }
    if(!changed) return false;
    // add the final segment points
    if(segmentPoints.size()>0) {
        newsegments.push_back(segmentPoints);
    }
    
    if(newsegments.size()==0) {
        //ofLogNotice("No line segments!");
        points.clear();
        colours.clear();
        return true;
    }
    
    // TODO break this out... also maybe split into separate polylines?
    points.clear();
    colours.clear();
       
    for(int i = 0; i<newsegments.size(); i++) {
        vector<Point> segpoints = newsegments[i];
        for(Point& p : segpoints) {
            points.push_back(p);
            colours.push_back(p.getColour());
        }
        // if we have another one...
        if(i+1<newsegments.size()) {
            ofColor blankColor = ofColor(0); // reversed?ofColor(0,255,255) : ofColor(255,0,0);
            points.push_back(segpoints.back() + glm::vec3(0.1,0.1,0));
            colours.push_back(blankColor);
            
            points.push_back(newsegments[i+1].front()+ glm::vec3(0.1,0.1,0));
            colours.push_back(blankColor);
        }
    }
    ofColor blankColor = ofColor(0,0,0);
    
    
    if(closed) {
        points.insert(points.begin(), points.front()+ glm::vec3(0.1,0.1,0));
        colours.insert(colours.begin(), blankColor);
    }
    
    points.push_back(points.back()+ glm::vec3(0.1,0.1,0));
    colours.push_back(blankColor);
    
        
    setDirty();
    
    return true;
    
    
}


vector<glm::vec3> Shape :: getLineRectangleIntersectionPoints(ofRectangle& rect, glm::vec3 p0, glm::vec3 p1) {
    
    glm::vec3 topLeft(rect.getTopLeft());
    glm::vec3 topRight(rect.getTopRight());
    glm::vec3 bottomRight(rect.getBottomRight());
    glm::vec3 bottomLeft(rect.getBottomLeft());
    vector<glm::vec3> intersections;
    glm::vec3 intersection;
    
    if(ofLineSegmentIntersection(p0, p1, topLeft,     topRight,    intersection)) {
        intersections.push_back(intersection);
    }
    
    if(ofLineSegmentIntersection(p0, p1, topRight,    bottomRight, intersection)) {
        intersections.push_back(intersection);
    }
    if(ofLineSegmentIntersection(p0, p1, bottomRight, bottomLeft,  intersection)) {
        intersections.push_back(intersection);
    }
    if(ofLineSegmentIntersection(p0, p1, bottomLeft,  topLeft,     intersection)) {
        intersections.push_back(intersection);
    }

    return intersections;
    
    
}


bool Shape :: doesLineIntersectRect(ofRectangle& rect, glm::vec3 p0, glm::vec3 p1) {
    
    glm::vec3 topLeft(rect.getTopLeft());
    glm::vec3 topRight(rect.getTopRight());
    glm::vec3 bottomRight(rect.getBottomRight());
    glm::vec3 bottomLeft(rect.getBottomLeft());
    
    glm::vec3 intersection;
    
    if(ofLineSegmentIntersection(p0, p1, topLeft,     topRight,    intersection)) {
        return true;
    }
    
    if(ofLineSegmentIntersection(p0, p1, topRight,    bottomRight, intersection)) {
        return true;
    }
    if(ofLineSegmentIntersection(p0, p1, bottomRight, bottomLeft,  intersection)) {
        return true;
    }
    if(ofLineSegmentIntersection(p0, p1, bottomLeft,  topLeft,     intersection)) {
        return true;
    }

    return false;
    
    
}


// want to return points that are on the edge of the rectangle, unlike oF rect.inside
bool Shape :: pointInsideRect(glm::vec3& p, ofRectangle& rect) {
    return rect.inside(p);
//    return p.x >= rect.getMinX() && p.y >= rect.getMinY() &&
//           p.x <= rect.getMaxX() && p.y <= rect.getMaxY();
}



bool Shape :: isFilled() const {
    return filled;
}

void Shape :: setFilled(bool state) {
    if(fillable && state) {
        filled = true;
    } else {
        filled = false;
    }
}
bool Shape :: isStroked() const {
    return stroked;
}
void Shape :: setStroked(bool strokestate) {
    stroked = strokestate;
}

void Shape :: setClosed(bool closestate) {
    if(closestate!=closed) {
        closed = closestate;
        lengthsDirty = true; // bounding box not dirty
    }
}

bool Shape :: isClosed() {
    return closed;
} 

float Shape ::getMedianZDepth() const {
    if(isEmpty()) return 0;
    if(points.size()==1) return points[0].z;
    float minZ = std::numeric_limits<float>::max();
    float maxZ = -10000000.0f;
    for(const glm::vec3&p : points) {
        if(p.z<minZ) minZ = p.z;
        if(p.z>maxZ) maxZ = p.z;
    }
    
    return ofMap(0.5,0,1,minZ, maxZ);
    
}
