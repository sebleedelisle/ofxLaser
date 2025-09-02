//
//  ofxLaserFactory.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 15/01/2019.
//
//

#pragma once
#include "ofMain.h"
#include "Poco/ObjectPool.h"

namespace ofxLaser {
class Factory {
	
	public:
	
	static Poco::ObjectPool<ofPolyline> polylineObjectPool;
	
	
	static void releasePolyline(ofPolyline* polyToRelease) {

		polylineObjectPool.returnObject( polyToRelease);
		
	}
	static ofPolyline* getPolyline(const ofPolyline& polyToClone) {
		return Factory::getPolyline(&polyToClone); 
	}
	static ofPolyline* getPolyline(const ofPolyline* polyToClone) {

		ofPolyline* poly;

		poly = polylineObjectPool.borrowObject();
		
        *poly = *polyToClone;
	
		return poly;
	}
    static ofPolyline* getPolyline(bool clear = true) {

        ofPolyline* poly;

        poly = polylineObjectPool.borrowObject();
        
        if(clear) poly->clear();
        
        return poly;
    }

	
	protected:

	private:

};

}
