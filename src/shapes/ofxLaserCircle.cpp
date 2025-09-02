//
//  ofxLaserCircle.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 16/11/2017.
//
//

#include "ofxLaserCircle.h"

using namespace ofxLaser;
//class Manager;
Circle::Circle(const ofPoint& _centre, const float _radius, const ofColor& col, string profilelabel){
	
	// seems like an over-engineered way of doing it but it's the only
	// way to ensure the transformations are taken into account.
	
	reversable = false;
	colour = col;
    radius = _radius;
    centre = _centre;
	
	polyline.clear();
	glm::vec3 p;
    
    // TODO fade out overlap
	for(int angle = -1; angle<=361; angle++) {
		p = glm::rotateZ(glm::vec3(radius, 0, 0), ofDegToRad(angle));
		//p.rotate(i, ofPoint(0,0,1));
		p+=centre;
        // projection is now done within the laser manager
		polyline.addVertex(p);
	}
	
	const vector<glm::vec3>& vertices = polyline.getVertices();
	
	startPos = vertices.front();
	
	endPos = vertices.back();
	
	tested = false;
	profileLabel = profilelabel;
	
}

void Circle::appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier){
	
	float acceleration = profile.acceleration;
	float speed = profile.speed;
	float length = polyline.getPerimeter();
	
	vector<float> unitDistances = getPointsAlongDistance(length, acceleration, speed, speedMultiplier);
	
	
	for(size_t i = 0; i<unitDistances.size(); i++) {
		
		ofPoint p = polyline.getPointAtLength((unitDistances[i]* length));
		
		points.push_back(ofxLaser::Point(p, colour));
	}
}

void Circle::addPreviewToMesh(ofMesh& mesh){
	
	
	auto & vertices = polyline.getVertices();
	mesh.addColor(ofColor(0));
	mesh.addVertex(vertices.front());
	
	for(size_t i = 0; i<vertices.size(); i++) {
		
		mesh.addColor(colour);
		mesh.addVertex(vertices[i]);
	}
	
	
	mesh.addColor(ofColor(0));
	mesh.addVertex(vertices.back());
}

bool Circle::intersectsRect(ofRectangle & rect) {
    
    
    const ofRectangle& bounds = polyline.getBoundingBox();
    
    if(rect.inside(bounds)) {
       //cout << "fast out " << true << endl;
        return true;
        
    }
    if(!rect.intersects(bounds)) {
       // cout << "fast out " << false << endl;
        return false;
    }
    
    vector<glm::vec3>& points = polyline.getVertices();
    
    for(glm::vec3& p : points) {
        if(rect.inside(p)) {
        //    cout << true << endl;
            return true;
        }
    }
   // cout << false << endl;
    return false;
	
};
