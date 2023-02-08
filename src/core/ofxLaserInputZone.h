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

	class InputZone : public QuadGui {
		
		public :
		
		InputZone(float x=0, float y=0, float w=800, float h=800);
		~InputZone();
        
        virtual void set(float x, float y, float w, float h) override ;
        virtual void draw() override;
        
        void setIndex(int _index);
        int getIndex(){ return index; };
		
		bool update();
        
        virtual bool deserialize(ofJson&jsonGroup) override;
        
	    ofRectangle& getRect() {
            return rect;
        } 
		
        string zoneLabel;
	
        protected : 
		int index = 0;
        ofRectangle rect;
		
	};
}
