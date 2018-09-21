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

namespace ofxLaser {
class Warper {
	
	public:
	
//	void setSrc(const ofRectangle& rect);
//	void setSrc(float x, float y, float w, float h) ;
//	void setDst(const ofRectangle& rect);
//	void setDst(float x, float y, float w, float h) ;
//	
	void updateHomography(glm::vec3 src1, glm::vec3 src2, glm::vec3 src3, glm::vec3 src4, glm::vec3 dst1, glm::vec3 dst2, glm::vec3 dst3, glm::vec3 dst4);
	
	Point getWarpedPoint(const Point& p);
	Point getUnWarpedPoint(const Point& p);
	glm::vec3 getWarpedPoint(const glm::vec3& p);
	glm::vec3 getUnWarpedPoint(const glm::vec3& p);
	
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
	
private:

};
	
}
