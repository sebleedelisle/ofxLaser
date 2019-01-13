//
//  ofxLaser :: Graphic
//
//  Created by Seb Lee-Delisle on 01/03/2018.
//
//

#pragma once
#include "ofMain.h"
#include "ofxLaserManager.h"
#include "ofxSvg.h"

namespace ofxLaser {

// contains vector of polylines and colours
// also a render function that sends them to the laser

class Graphic {
	
	public:
	Graphic() {
		Graphic::numGraphicsInMemory ++;
		ofLog(OF_LOG_NOTICE, "numGraphicsInMemory : " + ofToString(Graphic::numGraphicsInMemory));
	}
	
	~Graphic() {
		Graphic::numGraphicsInMemory --;
		
		for(ofPolyline* poly : polylines) {
			Graphic::releasePolyline(poly);
		}
	}
	// copy constructor
	Graphic(const Graphic &g) {
		Graphic::numGraphicsInMemory ++;
		colours = g.colours;
		for(ofPolyline* poly : g.polylines) {
			polylines.push_back(Graphic::getPolyline(poly));
		}
		
	}

	
	void addSvg(ofxSVG& svg, bool optimise = true);
	
	void addPolyline(const ofPolyline& poly, ofColor& colour, bool useTransform = true);
	void addPolylineUnder(const ofPolyline& poly, ofColor& colour, bool useTransform=true);
	
	void renderToLaser(ofxLaser::Manager& laser, float brightness = 1, string renderProfile = OFXLASER_PROFILE_DEFAULT);
	
    void clear();
    
	void autoCentre();
	void translate(glm::vec3 offset);
	
	// goes through all the polylines and connects touching lines
	// that are the same colour
	void connectLineSegments();
	
	// connects two polylines that are touching into one
	// poly1 is edited to contain the two shapes
	// returns false if lines aren't touching
	bool joinPolylines(ofPolyline& poly1, ofPolyline& poly2);
	
	// a boolean subtract operation
	void subtractShapeFromPolylines(const ofPolyline& polyToSubtract, vector<ofPolyline*>& polys, vector<ofColor>& colours);
	void subtractShapesFromPolylines(const vector<ofPolyline*>& polysToSubtract, vector<ofPolyline*>& polys, vector<ofColor>& colours);

	void subtractPathFromPolylines(ofPath& sourcepath, vector<ofPolyline*>& targetpolys, vector<ofColor>& colours);
	bool pointInsidePath(glm::vec3 point, ofPath& path);
	
	bool pointInsidePolylines(glm::vec3 point, const vector<ofPolyline*>& polys);

	void transformPolyline(ofPolyline& poly);
	
	glm::vec3 gLProject(glm::vec3& v);

	
	// returns angle of difference or 360 if they are not touching
	float comparePolylines(ofPolyline& poly1, ofPolyline& poly2);
	float getAngleBetweenPoints(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
	
	vector<ofPolyline*> polylines;
	vector<ofColor> colours;
	
	// for avoiding making and destroying polylines
	ofPolyline tempPoly;
	
	
	
	static vector<ofPolyline*> polylinePool;
	static vector<ofPolyline*> polylineSpares;
	static int numGraphicsInMemory;
	
	static bool releasePolyline(ofPolyline* polyToRelease) {
		//ofLog(OF_LOG_NOTICE, "polylinePool.size() : " + ofToString(polylinePool.size())+ " polylineSpares.size() : " + ofToString(polylineSpares.size()));
		vector<ofPolyline*>::iterator it = std::find( polylinePool.begin(), polylinePool.end(),polyToRelease);
		if(it==polylinePool.end()){
			ofLog(OF_LOG_WARNING, "ofxLaser::Graphic::releasePolyline - poly not found in pool!");
			return false;
		} else {
			ofPolyline* poly = *it;
			polylinePool.erase(it);
			polylineSpares.push_back(poly);
			return true;
		}
			
	}
	static ofPolyline* getPolyline(const ofPolyline* polyToClone = NULL) {
		ofLog(OF_LOG_NOTICE, "polylinePool.size() : " + ofToString(polylinePool.size())+ " polylineSpares.size() : " + ofToString(polylineSpares.size()));
		
		ofPolyline* poly;
		if(polylineSpares.size()>0) {
			poly = polylineSpares.back();
			polylineSpares.pop_back();
			poly->clear();
			
		} else {
			poly = new ofPolyline();
		}
		polylinePool.push_back(poly);
		if(polyToClone!=NULL) {
			*poly = *polyToClone;
		}
		
		return poly;
	}

	protected:

	private:

};

}
