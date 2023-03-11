//
//  ofxLaserZone.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#pragma once
#include "ofMain.h"
#include "ofxLaserZoneId.h"
#include "ofxLaserZoneIdContainer.h"

namespace ofxLaser {

	class InputZone : public ObjectWithZoneId {
		
		public :
		
		InputZone(float x=0, float y=0, float w=800, float h=800);
		~InputZone();
        
        virtual void set(float x, float y, float w, float h) ;
        //virtual void draw() override;
        
        void setZoneId(ZoneId zoneId);
        ZoneId& getZoneId(){ return zoneId; };
		
		bool update();
        
        virtual void serialize(ofJson&json) const override;
        virtual bool deserialize(ofJson&jsonGroup) override;
        
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
        ZoneId zoneId;
		
	};
}
