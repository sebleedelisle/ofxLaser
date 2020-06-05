//
// Created by Seb Lee-Delisle on 2019-02-10.
//
#pragma once
#include "ofMain.h"

class PolylineUtils {
	public :
	static bool getIntersectionPoints(ofPolyline& poly, glm::vec3 p1, glm::vec3 p2, vector<glm::vec3>& intersectionPoints);
	static bool getIntersectionPoints(vector<ofPolyline*>& polylines, glm::vec3 p1, glm::vec3 p2, vector<glm::vec3>& intersectionPoints);

};

