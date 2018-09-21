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
	
	void addSvg(ofxSVG& svg, bool optimise = true);
	
	void addPolyline(const ofPolyline& poly, ofColor& colour, glm::vec3 offset = glm::vec3());
	
	void renderToLaser(ofxLaser::Manager& laser, float brightness = 1, string renderProfile = OFXLASER_PROFILE_DEFAULT);
	
    void clear();
    
	void autoCentre();
	void translate(glm::vec3 offset);
	void connectLineSegments();
	bool joinPolylines(ofPolyline& poly1, ofPolyline& poly2);
	
	void subtractShapeFromPolylines(ofPolyline shape, vector<ofPolyline>& polys, vector<ofColor>& colours);
	void subtractPathFromPolylines(ofPath sourcepath, vector<ofPolyline>& targetpolys, vector<ofColor>& colours);
	bool pointInsidePath(glm::vec3 point, ofPath& path);

	// returns angle of difference or 360 if they are not touching
	float comparePolylines(ofPolyline& poly1, ofPolyline& poly2);
	float getAngleBetweenPoints(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
	
	vector<ofPolyline> polylines;
	vector<ofColor> colours;
	
	//ofRectangle boundingBox;
	
	//int targetZone = 0; 
	protected:

	//void updateBoundingBox();
	
	
	
	
	private:

};

}
