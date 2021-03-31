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
	static ofPolyline* getPolyline(const ofPolyline* polyToClone = NULL) {

		ofPolyline* poly;

		poly = polylineObjectPool.borrowObject();
		if(polyToClone!=NULL) {
			*poly = *polyToClone;
		} else {
			poly->clear();
		}
		
		return poly;
	}
	

	
	protected:

	private:

};

}
