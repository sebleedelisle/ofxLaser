//
//  ofxLaserZone.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#pragma once
#include "ofMain.h"
#include "ofxLaserShape.h"
#include "ofxLaserDot.h"
#include "ofxLaserQuadGui.h"
namespace ofxLaser {

	class Zone : public QuadGui {
		
		public :
		
		Zone();
		Zone(float x, float y, float w, float h);
		~Zone();
        
        virtual void set(float x, float y, float w, float h) override ;
        void setIndex(int _index);
		void setHandleSize(float size);
		
		bool update();
        // the zone object stores shapes that intersect with it
		bool addShape(Shape* s);
		//void draw();
		
		ofPoint& addSortedShapesToVector(vector<Shape*>& shapes, ofPoint& currentPosition);
	
		ofRectangle rect;

		deque<Shape*> shapes;

      //  bool editable;
	
    		
		int index = 0;
		
		
	};
}
