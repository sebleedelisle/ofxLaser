//
//  ofxLaserPolyline.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 16/11/2017.
//
//

#include "ofxLaserPolyline.h"
using namespace ofxLaser;


Polyline::Polyline() {
	
	reversable = true;
	colour = ofColor::white;
	cachedProfile = nullptr;
	multicoloured = false;
	
	tested = false;
	profileLabel = "";

}

Polyline::Polyline(const ofPolyline& poly, const ofColor& col, string profilelabel, float brightness){
	init(poly, col, profilelabel, brightness);
	
}

Polyline::Polyline(const ofPolyline& poly, const vector<ofColor>& sourcecolours, string profilelabel, float brightness){
	
	init(poly, sourcecolours, profilelabel, brightness);

}

Polyline::Polyline(const vector<glm::vec3>& points, const vector<ofColor>& colours, string profilelabel, float brightness) {
    init(points, colours, profilelabel, brightness);
    
}




void Polyline::init(const ofPolyline& poly, const ofColor& col, string profilelabel, float brightness){
	
	reversable = true;
	colour = col*brightness;
	cachedProfile = NULL;
	multicoloured = false;
	
	tested = false;
	profileLabel = profilelabel;
	
	initPoly(poly);
	
}

void Polyline::init(const ofPolyline& poly, const vector<ofColor>& sourcecolours, string profilelabel, float brightness){
	
	reversable = true;
	cachedProfile = NULL;
	
	multicoloured = true;
    colours.resize(sourcecolours.size());
    for(size_t i = 0; i<sourcecolours.size(); i++ ) {
        colours[i] = sourcecolours[i]*brightness;
    }
	//colours = sourcecolours; // should copy
	
	tested = false;
	profileLabel = profilelabel;
	
	
	initPoly(poly);
    
	
}

void Polyline::initPoly(const ofPolyline& poly) {
    
    initPoly(poly.getVertices());
    
    ofPolyline& polyline = *polylinePointer;
    if(poly.isClosed()) {
        polyline.addVertex(polyline.getVertices().front());
        polyline.setClosed(false);
    }
}


void Polyline::init(const vector<glm::vec3>& points, const vector<ofColor>& sourcecolours, string profilelabel, float brightness){
    
    reversable = true;
    cachedProfile = NULL;
    
    multicoloured = true;
    colours.resize(sourcecolours.size());
    for(size_t i = 0; i<sourcecolours.size(); i++ ) {
        colours[i] = sourcecolours[i]*brightness;
    }
    
    tested = false;
    profileLabel = profilelabel;
    
    initPoly(points); 
}

//void Polyline::initPoly(const ofPolyline& poly){
void Polyline::initPoly(const vector<glm::vec3> verticesToCopy){
   
	if(polylinePointer==NULL) {
		polylinePointer = ofxLaser::Factory::getPolyline(false);
	} else {
    	//polylinePointer->clear();
	}
    vector<glm::vec3>& vertices = polylinePointer->getVertices();
    //const vector<glm::vec3>& verticesToCopy = poly.getVertices();
    
    vertices.resize(verticesToCopy.size());
    
    //if(polylinePointer->getVertice)
	//*polylinePointer = poly;  // makes a copy, hopefully
    
    for(size_t i = 0; i<verticesToCopy.size(); i++) {

        glm::vec3& vertex = vertices[i];
        vertex = verticesToCopy[i];
        if(i==0) {
            boundingBox.set(vertex, 1,1);
        } else {
            boundingBox.growToInclude(vertex);
        }
    }
	
	
	//const vector<glm::vec3>& vertices = polyline.getVertices();
	
	startPos = vertices.front();
	// to avoid a bug in polyline in open polys
	endPos = vertices.back();
	//boundingBox = polyline.getBoundingBox();
   
	
}




Polyline:: ~Polyline() {
	// not sure if there's any point clearing the polyline - they should just get destroyed, right?
	//polyline.clear();
	if(polylinePointer!=NULL) {
		ofxLaser::Factory::releasePolyline(polylinePointer);
	}
}

void Polyline::appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier) {
	
	// TODO - take into account the speed multiplier!
	ofPolyline& polyline = *polylinePointer;
	if(&profile == cachedProfile) {
//		ofLog(OF_LOG_NOTICE, "cached points used");
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
	
	int numVertices =(int)polyline.size();
	while(endpoint<numVertices-1) {
		
		do {
			endpoint++;
		} while ((endpoint< (int)polyline.size()-1) && abs(polyline.getDegreesAtIndex(endpoint)) < cornerThresholdAngle);
		
		
		float startdistance = polyline.getLengthAtIndex(startpoint);
		float enddistance = polyline.getLengthAtIndex(endpoint);
		
		float length = enddistance - startdistance;
		
		ofPoint lastpoint;
		
		if(length>0) {
			
			vector<float>& unitDistances = getPointsAlongDistance(length, acceleration, speed, speedMultiplier);
			
			
			for(size_t i = 0; i<unitDistances.size(); i++) {
				
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
	
	ofPolyline& polyline = *polylinePointer;
	const vector<glm::vec3>& vertices = polyline.getVertices();
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
	ofPolyline& polyline = *polylinePointer;
	if(!rect.intersects(boundingBox)) return false;
	const vector<glm::vec3> & vertices = polyline.getVertices();
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
