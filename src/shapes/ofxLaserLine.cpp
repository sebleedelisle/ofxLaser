//
//  ofxLaserLine.cpp
//
//  Created by Seb Lee-Delisle on 01/08/2013.
//
//

#include "ofxLaserLine.h"


using namespace ofxLaser;

Line::Line(const ofPoint& startpos, const ofPoint& endpos, const ofColor& col, string profilelabel){

    reversable = true;
    colour = col;
    
    startPos = startpos;
    endPos = endpos;
    
    tested = false;
    profileLabel = profilelabel;
   
}


void Line::appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier) {
    

    ofPoint& start = getStartPos();
    ofPoint& end = getEndPos();
    ofVec2f v = end-start;

    float distanceTravelled = ofDist(start.x, start.y, end.x, end.y);
    vector<float>& unitDistances = getPointsAlongDistance(distanceTravelled, profile.acceleration, profile.speed, speedMultiplier);
    
    ofPoint p;
    
    for(size_t i = 0; i<unitDistances.size(); i++) {
        
        float unitDistance = unitDistances[i];
        
        points.push_back(ofxLaser::Point(start + (v*unitDistance), colour));
    }
};

void Line::addPreviewToMesh(ofMesh& mesh){
    mesh.addColor(ofColor(0));
    mesh.addVertex(getStartPos());
    
    mesh.addColor(colour);
    mesh.addVertex(getStartPos());
    mesh.addColor(colour);
    mesh.addVertex(getEndPos());
    
    mesh.addColor(ofColor(0));
    mesh.addVertex(getEndPos());
}

bool Line::intersectsRect(ofRectangle & rect) {
    // TODO fix intersection - openFrameworks line intersection has a bug
    return rect.intersects(startPos, endPos);
    
}
