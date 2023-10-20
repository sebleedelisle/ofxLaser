//
//  ofxLaserLine.cpp
//
//  Created by Seb Lee-Delisle on 01/08/2013.
//
//

#include "ofxLaserLine.h"


using namespace ofxLaser;

Line::Line(glm::vec3 startpos, glm::vec3 endpos, ofColor col, string profilelabel){

    reversable = true;
    colours = {col};
    fillable = false;
    
    points = {startpos, endpos};
    
    tested = false;
    profileLabel = profilelabel;
   
}


void Line::appendPointsToVector(vector<ofxLaser::Point>& pointsToAppendTo, const RenderProfile& profile, float speedMultiplier) {
    
    if(isEmpty()) return ;
    glm::vec3 start = getStartPos();
    glm::vec3 end = getEndPos();
    glm::vec3 v = end-start;

    float distanceTravelled = ofDist(start.x, start.y, end.x, end.y);
    vector<float>& unitDistances = getPointsAlongDistance(distanceTravelled, profile.acceleration, profile.speed, speedMultiplier);
    
    glm::vec3 p;
    
    for(size_t i = 0; i<unitDistances.size(); i++) {
        
        float unitDistance = unitDistances[i];
        // TODO interpolate if multicoloured
        pointsToAppendTo.push_back(ofxLaser::Point(start + (v*unitDistance), getColour()));
    }
};

void Line::addPreviewToMesh(ofMesh& mesh){
    if(isEmpty()) return ;
    mesh.addColor(ofColor(0));
    mesh.addVertex(getStartPos());
    
    mesh.addColor(getColour());
    mesh.addVertex(getStartPos());
    mesh.addColor(getColour());
    mesh.addVertex(getEndPos());
    
    mesh.addColor(ofColor(0));
    mesh.addVertex(getEndPos());
}

//bool Line::intersectsRect(ofRectangle & rect) {
//    // TODO fix intersection - openFrameworks line intersection has a bug
//    return rect.intersects(getStartPos(), getEndPos());
//    
//}


bool Line :: clipNearPlane(float nearPlaneZ) {
    if(isEmpty()) return false; 
    // if necessary, swap the points so that the endPos is further forward than the startPos
    glm::vec3& startPos = points[0];
    glm::vec3& endPos = points[1];
    
    if(startPos.z>endPos.z) {

        glm::vec3 temp = startPos;
        startPos = endPos;
        endPos = temp;
        
        
    }
    // if the startPos is still behind us then this line should be culled
    if(startPos.z>nearPlaneZ) {
        colours = {ofColor::black};
        return true;
    } else if(endPos.z>nearPlaneZ) {
        
      // trim it down
        
        float trimFactor = ofMap(nearPlaneZ, startPos.z, endPos.z, 0, 1);
        endPos = glm::mix(startPos, endPos, trimFactor);
        return true;
        
        
    } else {
        return false;
    }
    
    
}
