//
//  QuadWarp.cpp
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//

#include "ofxLaserQuadWarp.h"
using namespace ofxLaser;



void QuadWarp::setDst (const ofRectangle& rect) {
	setDst(rect.x, rect.y, rect.getWidth(), rect.getHeight());
}

void QuadWarp::setDst(float x, float y, float w, float h) {
	
    set(x, y, w, h);
   
	
}

void QuadWarp::setSrc (const ofRectangle& rect) {
	setSrc(rect.x, rect.y, rect.getWidth(), rect.getHeight());
}


void QuadWarp::setSrc (float x, float y, float w, float h) {
	
	srcPoints.clear();
	for(int i = 0; i<4; i++) {
		float xpos = ((float)(i%2)/1.0f*w)+x;
		float ypos = (floor((float)(i/2))/1.0f*h)+y;
		
		srcPoints.emplace_back(xpos, ypos);
	}
    srcPoints.emplace_back(x+w, y+h/2);
	
	
}


void QuadWarp :: draw() {
	
    
	if(isDirty) {
		updateHomography();
		
	}
    QuadGui::draw();
    
	
}

void QuadWarp::updateHomography() {
	
	// the source points are the zone points in screen space
	// the dest points are points in the projector space
	
	vector<cv::Point2f> srcCVPoints, dstCVPoints;
	

	for(int i = 0; i<4; i++) {
		srcCVPoints.push_back(cv::Point2f(srcPoints[i].x, srcPoints[i].y));
		dstCVPoints.push_back(cv::Point2f(handles[i].x, handles[i].y));
	}
	
	homography = cv::findHomography(cv::Mat(srcCVPoints), cv::Mat(dstCVPoints),CV_RANSAC, 100);
	inverseHomography = homography.inv();
}


ofxLaser::Point QuadWarp::getWarpedPoint(const ofxLaser::Point& p){
	
	if(post.size()<1) pre.resize(1);
	if(post.size()<1) pre.resize(1);
	pre[0].x = p.x;
	pre[0].y = p.y;
	
	
	cv::perspectiveTransform(pre, post, homography);
	ofxLaser::Point point =p;
	point.x = post[0].x;
	point.y = post[0].y;
	
	return point;
	
}


ofxLaser::Point QuadWarp::getUnWarpedPoint(const ofxLaser::Point& p){

	
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

ofPoint QuadWarp::getWarpedPoint(const ofPoint& p){
	
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


ofPoint QuadWarp::getUnWarpedPoint(const ofPoint& p){
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

