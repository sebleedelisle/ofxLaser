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
		
		//Zone();
		Zone(float x=0, float y=0, float w=800, float h=800);
		~Zone();
        
        virtual void set(float x, float y, float w, float h) override ;
        virtual void draw() override;
        
        void setIndex(int _index);
        int getIndex(){ return index; };
		void setHandleSize(float size);
		
		bool update();
        // the zone object stores shapes that intersect with it
		bool addShape(Shape* s);
		
		ofPoint& addSortedShapesToVector(vector<Shape*>& shapes, ofPoint& currentPosition);
	
		ofRectangle rect;
        string zoneLabel;
		deque<Shape*> shapes;

      //  bool editable;
	
        protected : 
		int index = 0;
		
		
	};
}
