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
	
    reset();

}

void Polyline::reset() {
    reversable = true;
    colours = {ofColor::white};
    tested = false;
    closed = false;
    // is this still used ?
    //cachedProfile = nullptr;
    
}

Polyline::Polyline(const ofPolyline& poly, const ofColor& col, string profilelabel, float brightness){
    reset();
    profileLabel = profilelabel;
    setPoints(poly);
    setColours((vector<ofColor>){col}, brightness);
    
    
}


Polyline::Polyline(const ofPolyline& poly, const vector<ofColor>& sourcecolours, string profilelabel, float brightness){
    reset();
    profileLabel = profilelabel;
    setPoints(poly);
    setColours(sourcecolours, brightness);
    
}

Polyline::Polyline(const vector<glm::vec3>& points, bool _closed, const vector<ofColor>& colours, string profilelabel, float brightness) {
    reset();
    profileLabel = profilelabel;
    setPoints(points, _closed);
    setColours(colours, brightness);
   
}

Polyline::Polyline(const ofPolyline& poly, const vector<ofFloatColor>& sourcecolours, string profilelabel, float brightness){
    
    reset();
    profileLabel = profilelabel;
    setPoints(poly);
    setColours(colours, brightness);
}

Polyline::Polyline(const vector<glm::vec3>& points, bool _closed, const vector<ofFloatColor>& colours, string profilelabel, float brightness) {
   
    reset();
    profileLabel = profilelabel;
    setPoints(points, _closed);
    setColours(colours, brightness);
}

Polyline:: ~Polyline() {
    //if(polylinePointer!=NULL) {
     //   ofxLaser::Factory::releasePolyline(polylinePointer);
    //}
}

//
//void Polyline::init(const ofPolyline& poly, const ofColor& col, string profilelabel, float brightness){
//
//	reversable = true;
//    colours = {col*brightness};
//	cachedProfile = NULL;
//
//	tested = false;
//	profileLabel = profilelabel;
//
//	initPoly(poly);
//
//}
//
//
//void Polyline::init(const ofPolyline& poly, const vector<ofColor>& sourcecolours, string profilelabel, float brightness){
//
//    vector<ofFloatColor> floatColours;
//    for(const ofColor& c : sourcecolours) {
//        floatColours.push_back(ofFloatColor(c));
//    }
//
//    init(poly, floatColours, profilelabel, brightness);
//
//}
//
//void Polyline::init(const vector<glm::vec3>& points, const vector<ofColor>& sourcecolours, string profilelabel, float brightness){
//
//    vector<ofFloatColor> floatColours;
//    for(const ofColor& c : sourcecolours) {
//        floatColours.push_back(ofFloatColor(c));
//    }
//
//    init(points, floatColours, profilelabel, brightness);
//
//}
//
//void Polyline::init(const ofPolyline& poly, const vector<ofFloatColor>& sourcecolours, string profilelabel, float brightness){
//
//	reversable = true;
//	cachedProfile = NULL;
//
//    colours.resize(sourcecolours.size());
//    for(size_t i = 0; i<sourcecolours.size(); i++ ) {
//        colours[i] = sourcecolours[i]*brightness;
//    }
//	//colours = sourcecolours; // should copy
//
//	tested = false;
//	profileLabel = profilelabel;
//
//
//	initPoly(poly);
//}
//
//
//void Polyline::init(const vector<glm::vec3>& points, const vector<ofFloatColor>& sourcecolours, string profilelabel, float brightness){
//
//    reversable = true;
//    cachedProfile = NULL;
//
//    multicoloured = true;
//    colours.resize(sourcecolours.size());
//    for(size_t i = 0; i<sourcecolours.size(); i++ ) {
//        colours[i] = sourcecolours[i]*brightness;
//    }
//
//    tested = false;
//    profileLabel = profilelabel;
//
//    initPoly(points);
//}
//

//
//void Polyline::initPoly(const ofPolyline& poly) {
//
//    initPoly(poly.getVertices());
//
//    ofPolyline& polyline = *polylinePointer;
//    if(poly.isClosed()) {
//        polyline.addVertex(polyline.getVertices().front());
//        polyline.setClosed(false);
//    }
//}
//
//
//
////void Polyline::initPoly(const ofPolyline& poly){
//void Polyline::initPoly(const vector<glm::vec3> verticesToCopy){
//
//	if(polylinePointer==NULL) {
//		polylinePointer = ofxLaser::Factory::getPolyline(false);
//	} else {
//    	//polylinePointer->clear();
//	}
//    vector<glm::vec3>& vertices = polylinePointer->getVertices();
//    //const vector<glm::vec3>& verticesToCopy = poly.getVertices();
//
//    vertices.resize(verticesToCopy.size());
//
//    //if(polylinePointer->getVertice)
//	//*polylinePointer = poly;  // makes a copy, hopefully
//
//    for(size_t i = 0; i<verticesToCopy.size(); i++) {
//
//        glm::vec3& vertex = vertices[i];
//        vertex = verticesToCopy[i];
//        if(i==0) {
//            boundingBox.set(vertex, 1,1);
//        } else {
//            boundingBox.growToInclude(vertex);
//        }
//    }
//
//
//	//const vector<glm::vec3>& vertices = polyline.getVertices();
//
//	startPos = vertices.front();
//	// to avoid a bug in polyline in open polys
//	endPos = vertices.back();
//	//boundingBox = polyline.getBoundingBox();
//
//
//}




void Polyline::appendPointsToVector(vector<ofxLaser::Point>& pointsToAppendTo, const RenderProfile& profile, float speedMultiplier) {
    
    if(isEmpty()) return;
	
	// TODO - remove points that are black
//	ofPolyline& polyline = *polylinePointer;
//	if(&profile == cachedProfile) {
////		ofLog(OF_LOG_NOTICE, "cached points used");
//		points.insert(points.end(), cachedPoints.begin(), cachedPoints.end());
//		return;
//	}

//	cachedProfile = &profile;
    vector<ofxLaser::Point> newPoints;
	
    
	float acceleration = profile.acceleration;
	float speed = profile.speed;
	float cornerThresholdAngle = profile.cornerThreshold;

	int startpoint = 0;
	int endpoint = 0;
	
	int numVertices = closed ? points.size()+1 : points.size();
    
	while(endpoint<numVertices) {
		
        // TODO curve angle checking!
		do {
			endpoint++;
		} while ((endpoint< (int)points.size()-1) && abs(getAngleAtIndexDegrees(endpoint)) < cornerThresholdAngle);

        //ofLogNotice("angle : ") << getAngleAtIndexDegrees(endpoint);
        //endpoint = numVertices;
        
		float startdistance = getLengthAtIndex(startpoint);
		float enddistance = getLengthAtIndex(endpoint);
		
		float length = enddistance - startdistance;
		
		glm::vec3 lastpoint;
        ofColor c;
		
        if(length>0) {
            
            vector<float>& unitDistances = getPointsAlongDistance(length, acceleration, speed, speedMultiplier);
 
            for(size_t i = 0; i<unitDistances.size(); i++) {
                
                float distanceAlongPoly = (unitDistances[i]*0.999* length) + startdistance;
                
                glm::vec3 p = getPointAtDistance(distanceAlongPoly);
                ofFloatColor colour = getColourAtDistance(distanceAlongPoly);

                newPoints.push_back(ofxLaser::Point(p, colour));
         
                lastpoint = p;
                
            }
            
        }
        
        startpoint=endpoint;
        
	}
    pointsToAppendTo.insert(pointsToAppendTo.end(), newPoints.begin(), newPoints.end());
	
}

void Polyline :: addPreviewToMesh(ofMesh& mesh){
    
    if(isEmpty()) return;
    
	//ofPolyline& polyline = *polylinePointer;
	//const vector<glm::vec3>& vertices = polyline.getVertices();
	mesh.addColor(ofColor(0));
	mesh.addVertex(points.front());
	
    int numPoints = closed ? points.size()+1 : points.size();
    
	for(size_t i = 0; i<numPoints; i++) {
        mesh.addColor(getColourAtPoint(i));
		mesh.addVertex(points[i%points.size()]);
	}
	
	
	mesh.addColor(ofColor(0));
	mesh.addVertex(points.back());
}



bool Polyline :: clipNearPlane(float nearPlaneZ) {
    
    vector<glm::vec3> & vertices = points;
    vector<Point> segmentPoints;
    vector<vector<Point>> newsegments;
      
    bool changed = false;
    bool behind = true;
    // go through each point
    for(int i = 0; i<vertices.size(); i++) {
        
        glm::vec3& p = vertices.at(i);
        
        // if it's behind us
        if(p.z>nearPlaneZ) {
            // and we're not already behind
            if(!behind) {
                behind = true;
                // if we already have points then add an end point
                // for the shape that is on the edge of the plane
                if(i>0) {
                    glm::vec3 previousPoint = vertices.at(i-1);
                    float trimFactor = ofMap(nearPlaneZ, previousPoint.z, p.z, 0, 1);
                    ofColor c = getColourAtFloatIndex(i-1+trimFactor);
                  
                    segmentPoints.push_back(Point(glm::mix(previousPoint, p, trimFactor), c));
                    
                    newsegments.push_back(segmentPoints);
                    
                    segmentPoints.clear();
                }
               
            }
            changed = true;
            // otherwise if we are already behind then we don't need to do
            // anything except ignore this point
        
        } else { // else if we are in front of the plan...
            // and we're currently behind...
            if(behind) {
                behind = false;
                // if we are not at the first point
                if(i>0) {
                    // then get the edge position
                    
                    glm::vec3 previousPoint = vertices.at(i-1);
                    float trimFactor = ofMap(nearPlaneZ, previousPoint.z, p.z, 0, 1);
                    
                    ofColor c = getColourAtFloatIndex(i-1+trimFactor);
   
                    segmentPoints.push_back(Point(glm::mix(previousPoint, p, trimFactor), c));
                    changed = true;
                }
            }
            // either way, add this next point because we are on screen
            segmentPoints.push_back(Point(p, getColourAtPoint(i)));
                    
        }
        
    }
    
    if(!changed) return false;
        
    // add the final segment points
    if(segmentPoints.size()>0) {
        newsegments.push_back(segmentPoints);
    }
    
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
            points.push_back(segpoints.back());
            colours.push_back(ofColor(0,0,0));
            
            points.push_back(newsegments[i+1].front());
            colours.push_back(ofColor(0,0,0));
        }
    }
    setDirty();
    
    return true;
}

//
//ofFloatColor& Polyline :: getColourAt(int index) {
//    if(!multicoloured) return getColour();
//    else {
//        index = ofClamp(index, 0, colours.size()-1);
//        return colours[index];
//    }
//
//}
