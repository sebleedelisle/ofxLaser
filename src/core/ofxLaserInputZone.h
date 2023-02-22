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

	class InputZone  {
		
		public :
		
		InputZone(float x=0, float y=0, float w=800, float h=800);
		~InputZone();
        
        virtual void set(float x, float y, float w, float h) ;
        //virtual void draw() override;
        
        void setIndex(int _index);
        int getIndex(){ return index; };
		
		bool update();
        
        virtual void serialize(ofJson&json) const ;
        virtual bool deserialize(ofJson&jsonGroup);
        
	    ofRectangle& getRect() {
            return rect;
        } 
		
        string zoneLabel;
        
//        const int numHandles = 4;
//        DragHandle handles[4];
        
        protected : 
		int index = 0;
        ofRectangle rect;

        bool isDirty = true;
		
	};
}
