//
//  ofxLaserCircle.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 16/11/2017.
//
//

#include "ofxLaserCircle.h"

using namespace ofxLaser;

Circle::Circle(const ofPoint& center, const float radius, const ofColor& col, string profilelabel){
	
	// seems like an over-engineered way of doing it but it's the only
	// way to ensure the transformations are taken into account.
	
	// TODO work out top, bottom, left and right points warped
	
	reversable = false;
	colour = col;
	
	
	polyline.clear();
	ofPoint p;
	for(int i = 0; i<360; i++) {
		p.set(radius, 0);
		p.rotate(i, ofPoint(0,0,1));
		p+=center;
		p = ofxLaser::Manager::instance()->gLProject(p);
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
	return rect.intersects(startPos, endPos);
	
};
