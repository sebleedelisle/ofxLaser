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
#include "ofxOpenCv.h"
#include "ofxLaserPoint.h"
#include "opencv2/calib3d/calib3d.hpp"


namespace ofxLaser {
class Warper {
	
	public:

	void updateHomography(glm::vec2 src1, glm::vec2 src2, glm::vec2 src3, glm::vec2 src4, glm::vec2 dst1, glm::vec2 dst2, glm::vec2 dst3, glm::vec2 dst4);

	glm::vec3 getWarpedPoint(const glm::vec3& p, bool useHomography = true);
	Point getWarpedPoint(const Point& p, bool useHomography = true);
	cv::Point2f getWarpedPoint(float x, float y, bool useHomography = true);

	Point getUnWarpedPoint(const Point& p, bool useHomography = true);
	glm::vec3 getUnWarpedPoint(const glm::vec3& p, bool useHomography = true);
	
	cv::Point2f toCv(glm::vec3 p) {
		return cv::Point2f(p.x, p.y);
	}
    cv::Point2f toCv(glm::vec2 p) {
        return cv::Point2f(p.x, p.y);
    }
	glm::vec3 toOf(cv::Point2f p) {
		return glm::vec3(p.x, p.y,0);
	}

	
	cv::Mat homography;
	cv::Mat inverseHomography;
	
	protected:
	
	vector<cv::Point2f> pre, post;
	vector<cv::Point2f> srcCVPoints, dstCVPoints;

private:

};
	
}