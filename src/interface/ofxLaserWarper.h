//
//  ofxLaserWarper.h
//  NewTransformTest
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#pragma once
#include "ofMain.h"
#include "ofxOpenCv.h"
#include "ofxLaserPoint.h"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/calib3d/calib3d_c.h"

namespace ofxLaser {
class Warper {
	
	public:
	
//	void setSrc(const ofRectangle& rect);
//	void setSrc(float x, float y, float w, float h) ;
//	void setDst(const ofRectangle& rect);
//	void setDst(float x, float y, float w, float h) ;
//	
	void updateHomography(glm::vec3 src1, glm::vec3 src2, glm::vec3 src3, glm::vec3 src4, glm::vec3 dst1, glm::vec3 dst2, glm::vec3 dst3, glm::vec3 dst4);

	glm::vec3 getWarpedPoint(const glm::vec3& p, bool useHomography = true);
	Point getWarpedPoint(const Point& p, bool useHomography = true);
	cv::Point2f getWarpedPoint(float x, float y, bool useHomography = true);

	Point getUnWarpedPoint(const Point& p, bool useHomography = true);
	glm::vec3 getUnWarpedPoint(const glm::vec3& p, bool useHomography = true);
	
	cv::Point2f toCv(glm::vec3 p) {
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
