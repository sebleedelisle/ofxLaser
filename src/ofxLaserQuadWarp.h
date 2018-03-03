//
//  ofxLaserQuadWarp.h
//
//  Created by Seb Lee-Delisle on 17/08/2015.
//
//

#pragma once

#include "ofxLaserQuadGui.h"
#include "ofxOpenCv.h"
#include "ofxLaserPoint.h"
#include "opencv2/calib3d/calib3d.hpp"



namespace ofxLaser {
    class QuadWarp : public QuadGui{
	
	public :
    QuadWarp() :QuadGui() {
          setSrc(0,0,0,0);
    };
    ~QuadWarp() {};
    QuadWarp(string labelname, string filename) : QuadGui() {
            setName(labelname, filename);
            
        }
		void setSrc(const ofRectangle& rect);
		void setSrc(float x, float y, float w, float h) ;
		void setDst(const ofRectangle& rect);
		void setDst(float x, float y, float w, float h) ;
		
	void draw();
	
	cv::Mat homography;
	cv::Mat inverseHomography;
	
	void updateHomography();
	
	Point getWarpedPoint(const Point& p);
	Point getUnWarpedPoint(const Point& p);
	ofPoint getWarpedPoint(const ofPoint& p);
	ofPoint getUnWarpedPoint(const ofPoint& p);
	
	vector<ofPoint> srcPoints;
	
	protected :
	
	vector<cv::Point2f> pre, post;


	
};
}
