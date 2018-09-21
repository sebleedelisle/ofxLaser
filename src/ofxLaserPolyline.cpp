//
//  ofxLaserPolyline.cpp
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 16/11/2017.
//
//

#include "ofxLaserPolyline.h"
using namespace ofxLaser;


Polyline::Polyline(const ofPolyline& poly, const ofColor& col, string profilelabel){
	
	reversable = false;
	colour = col;
	cachedProfile = NULL;
	multicoloured = false;
	
	tested = false;
	profileLabel = profilelabel;
	
	
	initPoly(poly);
	
}

Polyline::Polyline(const ofPolyline& poly, const vector<ofColor>& sourcecolours, string profilelabel){
	
	reversable = false;
	cachedProfile = NULL;
	
	multicoloured = true;
	colours = sourcecolours; // should copy
	
	tested = false;
	profileLabel = profilelabel;
	
	
	initPoly(poly);
	

}



void Polyline::initPoly(const ofPolyline& poly){
    polyline.clear();
    auto & vertices =poly.getVertices();
    ofPoint p;
    for(size_t i = 0; i<vertices.size(); i++) {
        
        //p = ofxLaser::Manager::instance()->gLProject(vertices[i]);
        p = vertices[i];
        polyline.addVertex(p);
        
    }
   // if(poly.isClosed()) polyline.addVertex(vertices[0]); 
    polyline.setClosed(false);
	startPos = polyline.getVertices().front();
	// to avoid a bug in polyline in open polys
	endPos = polyline.getVertices().back();
   
	
}




Polyline:: ~Polyline() {
	// not sure if there's any point clearing the polyline - they should just get destroyed, right?
	polyline.clear();
}

void Polyline::appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier) {
	
	if(&profile == cachedProfile) {
		
		points.insert(points.end(), cachedPoints.begin(), cachedPoints.end());
		return;
	}
	
	cachedProfile = &profile;
	cachedPoints.clear();
	
	float acceleration = profile.acceleration;
	float speed = profile.speed;
	float cornerThresholdAngle = profile.cornerThreshold;

	int startpoint = 0;
	int endpoint = 0;
	
	int numVertices =(int)polyline.getVertices().size();
	while(endpoint<numVertices-1) {
		
		do {
			endpoint++;
		} while ((endpoint< (int)polyline.getVertices().size()-1) && abs(polyline.getDegreesAtIndex(endpoint)) < cornerThresholdAngle);
		
		
		float startdistance = polyline.getLengthAtIndex(startpoint);
		float enddistance = polyline.getLengthAtIndex(endpoint);
		
		float length = enddistance - startdistance;
		
		ofPoint lastpoint;
		
		if(length>0) {
			
			vector<float>& unitDistances = getPointsAlongDistance(length, acceleration, speed, speedMultiplier);
			
			
			for(int i = 0; i<unitDistances.size(); i++) {
				
				float distanceAlongPoly = (unitDistances[i]*0.999* length) + startdistance;
				
				ofPoint p = polyline.getPointAtLength(distanceAlongPoly);
				
				if(multicoloured) {
					int colourindex = round(polyline.getIndexAtLength(distanceAlongPoly)); // TODO - interpolate?
					colourindex = ofClamp(colourindex, 0,colours.size());
					cachedPoints.push_back(ofxLaser::Point(p, colours[colourindex]));
					
				} else {
					
					cachedPoints.push_back(ofxLaser::Point(p, colour));
				}
				
				lastpoint = p;
			}
			
		}
		
		startpoint=endpoint;
		
	}
	points.insert(points.end(), cachedPoints.begin(), cachedPoints.end());
	
}

void Polyline :: addPreviewToMesh(ofMesh& mesh){
	
	
	auto & vertices = polyline.getVertices();
	mesh.addColor(ofColor(0));
	mesh.addVertex(vertices.front());
	
	for(size_t i = 0; i<vertices.size(); i++) {
		
		if(multicoloured) {
			int colourindex = ofClamp(i, 0, colours.size()-1);
			mesh.addColor(colours[colourindex]);
		} else {
			mesh.addColor(colour);
		}
		mesh.addVertex(vertices[i]);
	}
	
	
	mesh.addColor(ofColor(0));
	mesh.addVertex(vertices.back());
}


bool Polyline:: intersectsRect(ofRectangle & rect){
	
	if(!rect.intersects(polyline.getBoundingBox())) return false;
	auto & vertices = polyline.getVertices();
	for(size_t i = 1; i< vertices.size(); i++) {
		if(rect.intersects(vertices[i-1],vertices[i])) return true;
	}
	return false;
	
}


//
//void Polyline::transformPolyline(const ofPolyline& source, ofPolyline& target, ofPoint pos, float rotation, ofPoint scale){
//	
//	target = source;
//	for(int i = 0; i<target.size(); i++) {
//		ofPoint& p = target[i];
//		p*=scale;
//		p.rotate(rotation, ofPoint(0,0,1));
//		p+=pos;
//		
//	}
//	
//	
//}
