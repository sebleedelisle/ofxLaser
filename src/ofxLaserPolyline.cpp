//
//  ofxLaserPolyline.cpp
//  LaserLightSynths2017c
//
//  Created by Seb Lee-Delisle on 12/02/2017.
//
//

#include "ofxLaserPolyline.h"

using namespace ofxLaser;

Polyline :: Polyline(const ofPolyline& sourcepoly, ofFloatColor polyColour, float polyAcceleration, float polySpeed, float polyCornerThresholdAngle, ofPoint pos = ofPoint::zero(), float rotation = 0, ofPoint scale = ofPoint::one()){
    
    // to do - implement reversibility!
    reversable = false;
    
    //polyline.clear();
    
    transformPolyline(sourcepoly, polyline, pos, rotation, scale); // makes a copy
    colour = polyColour;
    
    
    startPos = polyline.getVertices().front();
    // to avoid a bug in polyline in open polys
    endPos = polyline.getVertices().back();
    
    acceleration = polyAcceleration;
    speed = polySpeed;
    //previewMesh.setMode(OF_PRIMITIVE_LINES);
    cornerThresholdAngle = polyCornerThresholdAngle;
    
}

void Polyline::transformPolyline(const ofPolyline& source, ofPolyline& target, ofPoint pos, float rotation, ofPoint scale) {
    
    target = source;
    for(int i = 0; i<target.size(); i++) {
        ofPoint& p = target[i];
        p*=scale;
        p.rotate(rotation, ofPoint(0,0,1));
        p+=pos;
        p = ofxLaser::Manager::gLProject(p);
    }
    
}

void Polyline::appendPointsToVector(vector<ofxLaser::Point>& points) {
    
    ofPolyline& poly = polyline;
    int startpoint = 0;
    int endpoint = 0;
    
    while(endpoint<(int)poly.getVertices().size()-1) {
        
        do {
            endpoint++;
        } while ((endpoint< (int)poly.getVertices().size()-1) && abs(poly.getAngleAtIndex(endpoint)) < cornerThresholdAngle);
        
        
        float startdistance = poly.getLengthAtIndex(startpoint);
        float enddistance = poly.getLengthAtIndex(endpoint);
        
        float length = enddistance - startdistance;
        
        ofPoint lastpoint;
        
        if(length>0) {
            
            vector<float> unitDistances = getPointsAlongDistance(length, acceleration, speed);
            
            ofColor pointcolour;
            
            for(int i = 0; i<unitDistances.size(); i++) {
                ofPoint p = poly.getPointAtLength((unitDistances[i]*0.999* length) + startdistance);
                
                //pointcolour = laserpoly.getColourForPoint(unitDistances[i], p);
                
                //addIldaPoint(p, pointcolour, laserpoly.intensity);
                points.push_back(ofxLaser::Point(p, colour));
                // TODO reimplement preview mesh
                //					if(i>0) {
                //						laserpoly.previewMesh.addVertex(lastpoint);
                //						laserpoly.previewMesh.addVertex(p);
                //						laserpoly.previewMesh.addColor(pointcolour*laserpoly.intensity);
                //						laserpoly.previewMesh.addColor(pointcolour*laserpoly.intensity);
                //						//cout << lastpoint << " " << p << " " << pointcolour << endl;
                //
                //					}
                
                lastpoint = p;
            }
            
        }
        
        startpoint=endpoint;
        
    }
    
    
}
