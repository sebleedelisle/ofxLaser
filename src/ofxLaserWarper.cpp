//
//  ofxLaserWarper.cpp
//  NewTransformTest
//
//  Created by Seb Lee-Delisle on 07/02/2018.
//
//

#include "ofxLaserWarper.h"

using namespace ofxLaser; 




void Warper::updateHomography(ofPoint src1, ofPoint src2, ofPoint src3, ofPoint src4, ofPoint dst1, ofPoint dst2, ofPoint dst3, ofPoint dst4) {
	
	// the source points are the zone points in screen space
	// the dest points are points in the projector space
	
	vector<cv::Point2f> srcCVPoints, dstCVPoints;
	srcCVPoints.push_back(toCv(src1));
	srcCVPoints.push_back(toCv(src2));
	srcCVPoints.push_back(toCv(src3));
	srcCVPoints.push_back(toCv(src4));
	dstCVPoints.push_back(toCv(dst1));
	dstCVPoints.push_back(toCv(dst2));
	dstCVPoints.push_back(toCv(dst3));
	dstCVPoints.push_back(toCv(dst4));
	
	homography = cv::findHomography(cv::Mat(srcCVPoints), cv::Mat(dstCVPoints),CV_RANSAC, 100);
	inverseHomography = homography.inv();
}


ofPoint Warper::getWarpedPoint(const ofPoint& p){
	
	if(post.size()<1) pre.resize(1);
	if(post.size()<1) pre.resize(1);
	pre[0].x = p.x;
	pre[0].y = p.y;
	
	
	// TODO
	cv::perspectiveTransform(pre, post, homography);
	ofPoint point =p;
	point.x = post[0].x;
	point.y = post[0].y;
	
	return point;
	
	
	
}


ofPoint Warper::getUnWarpedPoint(const ofPoint& p){
	if(post.size()<1) pre.resize(1);
	if(post.size()<1) pre.resize(1);
	pre[0].x = p.x;
	pre[0].y = p.y;
	
	cv::perspectiveTransform(pre, post, inverseHomography);
	ofPoint point =p;
	point.x = post[0].x;
	point.y = post[0].y;
	
	return point;
	
	
	
}


ofxLaser::Point Warper::getWarpedPoint(const ofxLaser::Point& p){
	
	if(post.size()<1) pre.resize(1);
	if(post.size()<1) pre.resize(1);
	pre[0].x = p.x;
	pre[0].y = p.y;
	
	
    //cv::
	cv::perspectiveTransform(pre, post, homography);
	ofxLaser::Point point =p;
	point.x = post[0].x;
	point.y = post[0].y;
	
	return point;
	
	
	
}


ofxLaser::Point Warper::getUnWarpedPoint(const ofxLaser::Point& p){
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
