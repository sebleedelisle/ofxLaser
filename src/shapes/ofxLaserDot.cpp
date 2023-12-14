
//
//  LaserDot.h
//
//  Created by Seb Lee-Delisle on 20/05/2021.
//
//

#include "ofxLaserDot.h"
using namespace ofxLaser;

Dot::Dot(const glm::vec3& dotPosition, const ofColor& dotColour, float dotIntensity, string profilelabel){
    points = {dotPosition};
    colours = {dotColour};

    intensity = dotIntensity;
    tested = false;
    profileLabel = profilelabel;
    fillable = false; 
    
}

void Dot::appendPointsToVector(vector<ofxLaser::Point>& pointsToAppendTo, const RenderProfile& profile, float speedMultiplier) {
    int maxPoints = profile.dotMaxPoints;
    int pointcount = ceil(maxPoints * intensity/speedMultiplier);
    
    // interpolation aims to smooth the final point in the dot to have greater granularity of colours.
    // the value is the fraction of the brightness of the last pixel, from 0 (brightest) to 1(dimmest);
    float interpolation = pointcount - (maxPoints * intensity/speedMultiplier);
    ofColor col(getColour());
    
    for(int i = 0; i<pointcount-1; i++) {
        pointsToAppendTo.push_back(ofxLaser::Point(getStartPos(), col));
    }
    pointsToAppendTo.push_back(ofxLaser::Point(getStartPos(), col* ofMap(interpolation, 0, 1,1,0)));
};


void Dot::addPreviewToMesh(ofMesh& mesh){
    float radius = ofMap(intensity, 0, 1,0.1,0.8, true);
    ofColor c(getColour());
    c*=(ofMap(intensity,0,0.5,0,1,true));
    
    ofVec3f v(0,-radius);
    mesh.addColor(ofColor(0));
    mesh.addVertex(v + getStartPos());
    
    // draw a little circle
    for(int i = 0; i<=360; i+=30) {
        v.set(0, -radius);
        v.rotate(i, ofVec3f(0,0,1));
        mesh.addColor(c);
        mesh.addVertex(v+getStartPos());
    }
    
    v.set(0, -radius);
    mesh.addColor(ofColor(0));
    mesh.addVertex(v+getStartPos());
}


bool Dot :: clipNearPlane(float nearPlaneZ) {
    
    if(getStartPos().z>nearPlaneZ) {
        points.clear();
        colours.clear(); 
        colours = {ofColor::black};
        return true;
    } else {
        return false;
    }
    
}
