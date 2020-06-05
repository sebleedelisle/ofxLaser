//
// Created by Seb Lee-Delisle on 2019-02-10.
//

#include "PolylineUtils.h"

bool PolylineUtils::getIntersectionPoints(ofPolyline& poly, glm::vec3 p1, glm::vec3 p2, vector<glm::vec3>& intersectionPoints) {
	
	bool intersects = false;
	
	const vector<glm::vec3> polypoints = poly.getVertices();
	
	glm::vec3 lastPoint;
	
	for(int i = 0; i<polypoints.size(); i++ ){
		
		if(i>0){
			glm::vec3 intersection;
			const glm::vec3& currentpoint = polypoints[i];
			
			if (ofLineSegmentIntersection(lastPoint, currentpoint, p1, p2, intersection)) {
				intersectionPoints.push_back(intersection);
				intersects = true;
			}
			lastPoint = currentpoint;
		} else {
			lastPoint = polypoints[0];
		}
		
		
		
	}
	return intersects;
	
}
bool PolylineUtils::getIntersectionPoints(vector<ofPolyline*>& polylines, glm::vec3 p1, glm::vec3 p2, vector<glm::vec3>& intersectionPoints) {
	
	bool intersects = false;
	for(ofPolyline* poly : polylines) {
		if(getIntersectionPoints(*poly, p1, p2, intersectionPoints)) {
			intersects = true;
		}
		
	}
	return intersects;
	
}
