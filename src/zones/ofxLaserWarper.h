//
//  ofxLaserWarper.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//
// Used by the zone transform objects to converts points from input zone to
// output zone coordinates. It has no interface elements, they are provided
// by the zone transform object itself.
//

#pragma once
#include "ofMain.h"
#include "ofxLaserPoint.h"
#include <array>


namespace ofxLaser {
class Warper {
	
	public:

	void updateHomography(glm::vec2 src1, glm::vec2 src2, glm::vec2 src3, glm::vec2 src4, glm::vec2 dst1, glm::vec2 dst2, glm::vec2 dst3, glm::vec2 dst4);

	glm::vec3 getWarpedPoint(const glm::vec3& p, bool useHomography = true);
	Point getWarpedPoint(const Point& p, bool useHomography = true);
	glm::vec2 getWarpedPoint(float x, float y, bool useHomography = true);

	Point getUnWarpedPoint(const Point& p, bool useHomography = true);
	glm::vec3 getUnWarpedPoint(const glm::vec3& p, bool useHomography = true);

	
	glm::mat3 homography = glm::mat3(1.0f);
	glm::mat3 inverseHomography = glm::mat3(1.0f);
	bool homographyValid = false;
	
	protected:
	
	std::array<glm::vec2, 4> srcPoints{};
	std::array<glm::vec2, 4> dstPoints{};

private:
	glm::vec2 applyHomography(const glm::mat3& matrix, const glm::vec2& point) const;
	glm::vec2 getWarpedBilinearPoint(float x, float y) const;

};
	
}
