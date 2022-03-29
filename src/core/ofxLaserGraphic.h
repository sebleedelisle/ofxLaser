//
//  ofxLaser :: Graphic
//
//  Created by Seb Lee-Delisle on 01/03/2018.
//
//

#pragma once
#include "ofMain.h"
//#include "ofxLaserManagerBase.h"
#include "ofxClipper.h"
#include "ofxLaserFactory.h"
#include "ofxSvgExtra.h"

namespace ofxLaser {


class ManagerBase;

// contains vector of polylines and colours
class Graphic {
	
	public:
	Graphic() {
		Graphic::numGraphicsInMemory ++;
		//ofLog(OF_LOG_NOTICE, "numGraphicsInMemory : " + ofToString(Graphic::numGraphicsInMemory));
	}
	
	~Graphic() {
		Graphic::numGraphicsInMemory --;
		
		for(ofPolyline* poly : polylines) {
			Factory::releasePolyline(poly);
		}
	}
	// copy constructor
	Graphic(const Graphic &g) {
		Graphic::numGraphicsInMemory ++;
		colours = g.colours;
		for(ofPolyline* poly : g.polylines) {
			polylines.push_back(Factory::getPolyline(poly));
		}
		
	}

	
   void addSvg(string filename, bool optimise = true, bool subtractFills = true) {
        addSvgFromFile(filename, optimise, subtractFills);
    }
        
    void addSvgFromFile(string filename, bool optimise = true, bool subtractFills = true);
    void addSvgFromString(string data, bool optimise = true, bool subtractFills = true);
    void addSvg(ofxSVGExtra& svg, bool optimise = true, bool subtractFills = true);
	void addPolyline(const ofPolyline* poly, ofColor colour, bool filled = true, bool useTransform = true){
		addPolyline(*poly, colour, filled, useTransform);
	}
	void addPolyline(const ofPolyline& poly, ofColor colour, bool filled = true, bool useTransform = true);
	void addPath(const ofPath& path, bool useTransform = true, bool subtractFills = true, bool optimise = false);

	// subtract polyline shape from everything underneath
	void subtractPolyline(ofPolyline* polyToSubtract, bool useTransform = false);
	
	void intersectRect(ofRectangle& rect);
	void intersectPaths(vector<ofPath>& paths);

	
	// ensures a tiny gap in a closed poly so that line subtraction
	// works properly
	void breakPolyline(ofPolyline* poly);
	
	void replacePolylines(vector<ofPolyline*>& newpolys, vector<ofColor>&newcolours);

	
	
    void clear();
    
	void autoCentre();
	void translate(glm::vec3 offset);
	
	void serialize(ofJson&json);
	void deserialize(ofJson&json);
	void serializePoly(ofJson& json, ofPolyline& poly);
	void deserializePoly(ofJson& json, ofPolyline& poly);

	// goes through all the polylines and connects touching lines
	// that are the same colour
	void connectLineSegments();
	
	// connects two polylines that are touching into one
	// poly1 is edited to contain the two shapes
	// returns false if lines aren't touching
	bool joinPolylines(ofPolyline& poly1, ofPolyline& poly2);
	
	void subtractPathFromPolylines(ofPath& sourcepath);
	bool pointInsidePath(glm::vec3 point, ofPath& path);
	
	bool pointInsidePolylines(glm::vec3 point, const vector<ofPolyline*>& polys);

	void transformPolyline(ofPolyline& poly);
	ofPath transformPath(ofPath& path);
	
	glm::vec3 gLProject(glm::vec3& v);
	
	// returns angle of difference or 360 if they are not touching
	float comparePolylines(ofPolyline& poly1, ofPolyline& poly2);
	float getAngleBetweenPoints(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
    
    // ----------------------- DEPRECATED ------------------------------
    
    OF_DEPRECATED_MSG("ofxLaser::Graphic::renderToLaser(laser, brightness, profile) - has been moved into the LaserManager class - use laserManager.drawLaserGraphic(graphic);", void renderToLaser(ofxLaser::ManagerBase& laser, float brightness = 1, string renderProfile = ""));
	
	vector<ofPolyline*> polylines;
	vector<ofColor> colours;
	
	// a shape that represents all of the filled shapes - the "alpha channel"
	vector<ofPolyline> polylineMask;
	
	ofx::Clipper clipper;
	
	//object factory for ofPolylines!

	static int numGraphicsInMemory;


	protected:

	private:

};

}
