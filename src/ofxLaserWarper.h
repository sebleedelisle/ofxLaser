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
	void updateHomography(ofPoint src1, ofPoint src2, ofPoint src3, ofPoint src4, ofPoint dst1, ofPoint dst2, ofPoint dst3, ofPoint dst4);
	
	Point getWarpedPoint(const Point& p);
	Point getUnWarpedPoint(const Point& p);
	ofPoint getWarpedPoint(const ofPoint& p);
	ofPoint getUnWarpedPoint(const ofPoint& p);
	
	cv::Point2f toCv(ofPoint p) {
		return cv::Point2f(p.x, p.y);
	}
	ofPoint toOf(cv::Point2f p) {
		return ofPoint(p.x, p.y);
	}

	
	cv::Mat homography;
	cv::Mat inverseHomography;
	
	protected:
	
	vector<cv::Point2f> pre, post;
	
private:

};
	
}
