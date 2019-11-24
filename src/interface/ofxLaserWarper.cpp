//
//  ofxLaserWarper.cpp
//  NewTransformTest
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserWarper.h"

using namespace ofxLaser; 




void Warper::updateHomography(glm::vec3 src1, glm::vec3 src2, glm::vec3 src3, glm::vec3 src4, glm::vec3 dst1, glm::vec3 dst2, glm::vec3 dst3, glm::vec3 dst4) {
	
	// the source points are the zone points in screen space
	// the dest points are points in the projector space
	
	srcCVPoints.resize(4);
	dstCVPoints.resize(4);
	srcCVPoints[0] = toCv(src1);
	srcCVPoints[1] = toCv(src2);
	srcCVPoints[2] = toCv(src3);
	srcCVPoints[3] = toCv(src4);
	dstCVPoints[0] = toCv(dst1);
	dstCVPoints[1] = toCv(dst2);
	dstCVPoints[2] = toCv(dst3);
	dstCVPoints[3] = toCv(dst4);


	try{
	homography = cv::findHomography(cv::Mat(srcCVPoints), cv::Mat(dstCVPoints),CV_RANSAC, 100);
		inverseHomography = homography.inv();
	} catch ( cv::Exception & e ) {
		ofLog(OF_LOG_ERROR, e.msg ); // output exception message
	}
}


glm::vec3 Warper::getWarpedPoint(const glm::vec3& p, bool useHomography){


	cv::Point2f cvp = getWarpedPoint(p.x, p.y, useHomography);
	return glm::vec3(cvp.x, cvp.y, 0);



}



ofxLaser::Point Warper::getWarpedPoint(const ofxLaser::Point& p, bool useHomography){

	cv::Point2f cvp = getWarpedPoint(p.x, p.y, useHomography);
	ofxLaser::Point lp = p;
	lp.x = cvp.x;
	lp.y = cvp.y;
	return lp;

}
cv::Point2f Warper :: getWarpedPoint(float x, float y, bool useHomography) {


	if(useHomography) {
		if (post.size() < 1) post.resize(1);
		if (pre.size() < 1) pre.resize(1);
		pre[0].x = x;
		pre[0].y = y;
		try {
			cv::perspectiveTransform(pre, post, homography);
		} catch ( cv::Exception & e ) {
			ofLog(OF_LOG_ERROR, e.msg ); // output exception message
		}
		return post[0];
	} else {


//		P is the linear interpolation of A and B in u: P = A + (B-A)·u
//		Q is the linear interpolation of D and C in u: Q = D + (C-D)·u
//		X is the linear interpolation of P and Q in v: X = P + (Q-P)·v
//
//				therefore
//
//		X(u,v) = A + (B-A)·u + (D-A)·v + (A-B+C-D)·u·v


		cv::Point2f d = srcCVPoints[3] - srcCVPoints[0];
		float u = (x-(srcCVPoints[0].x))/d.x;
		float v = (y-(srcCVPoints[0].y))/d.y;
		cv::Point2f& A = dstCVPoints[0];
		cv::Point2f& B = dstCVPoints[1];
		cv::Point2f& C = dstCVPoints[3];
		cv::Point2f& D = dstCVPoints[2];

		return A + (B-A)*u + (D-A)*v + (A-B+C-D)*u*v;

	}
}


glm::vec3 Warper::getUnWarpedPoint(const glm::vec3& p, bool useHomography){



	if (post.size() < 1) pre.resize(1);
	if (post.size() < 1) pre.resize(1);
	pre[0].x = p.x;
	pre[0].y = p.y;

	cv::perspectiveTransform(pre, post, inverseHomography);
	ofPoint point = p;
	point.x = post[0].x;
	point.y = post[0].y;

	return point;

	
}


ofxLaser::Point Warper::getUnWarpedPoint(const ofxLaser::Point& p, bool useHomography){
	if(post.size()<1) pre.resize(1);
	if(post.size()<1) pre.resize(1);
	pre[0].x = p.x;
	pre[0].y = p.y;
	
	cv::perspectiveTransform(pre, post, inverseHomography);
	ofxLaser::Point point =p;
	point.x = post[0].x;
	point.y = post[0].y;
	
	return point;
	
	
	
}
