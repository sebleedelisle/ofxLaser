//
//  ofxLaserFactory.h
//  LaserLightSynths2019
//
//  Created by Seb Lee-Delisle on 15/01/2019.
//
//

#pragma once
#include "ofMain.h"
#include "ofxLaserCircle.h"
#include "Poco/ObjectPool.h"

//#include "ofxLaserShape.h"

namespace ofxLaser {
class Factory {
	
	public:
	
//	static vector<ofPolyline*> polylinePool;
//	static vector<ofPolyline*> polylineSpares;
	static Poco::ObjectPool<ofPolyline> polylineObjectPool;
	
	
	static bool releasePolyline(ofPolyline* polyToRelease) {

		polylineObjectPool.returnObject( polyToRelease);
		
		//
//		vector<ofPolyline*>::iterator it = std::find( polylinePool.begin(), polylinePool.end(),polyToRelease);
//		if(it==polylinePool.end()){
//			ofLog(OF_LOG_WARNING, "ofxLaser::Graphic::releasePolyline - poly not found in pool!");
//
//			return false;
//		} else {
//			ofPolyline* poly = *it;
//			polylinePool.erase(it);
//			polylineSpares.push_back(poly);
//
//			return true;
//		}
		
	}
	static ofPolyline* getPolyline(const ofPolyline& polyToClone) {
		return Factory::getPolyline(&polyToClone); 
	}
	static ofPolyline* getPolyline(const ofPolyline* polyToClone = NULL) {

		ofPolyline* poly;
//		if(polylineSpares.size()>0) {
//			poly = polylineSpares.back();
//			polylineSpares.pop_back();
//			poly->clear();
//
//		} else {
//			poly = new ofPolyline();
//		}
//		polylinePool.push_back(poly);

		poly = polylineObjectPool.borrowObject();
		if(polyToClone!=NULL) {
			*poly = *polyToClone;
		} else {
			poly->clear();
		}
		
		return poly;
	}
	
//
//	static Polyline* getLaserPolyline(const Polyline* polyToClone = NULL) {
//		Polyline* poly;
//		if(laserPolylineSpares.size()>0) {
//			poly = laserPolylineSpares.back();
//			laserPolylineSpares.pop_back();
//			poly->clear();
//
//		} else {
//			poly = new Polyline();
//		}
//		laserPolylinePool.push_back(poly);
//
//		if(polyToClone!=NULL) {
//			*poly = *polyToClone;
//		}
//
//		return poly;
//
//	}
//
//	static bool releaseLaserPolyline(Polyline* polyToRelease) {
//		vector<Polyline*>::iterator it = std::find( laserPolylinePool.begin(), laserPolylinePool.end(),polyToRelease);
//		if(it==laserPolylinePool.end()){
//			ofLog(OF_LOG_WARNING, "ofxLaser::Graphic::releasePolyline - poly not found in pool!");
//			return false;
//		} else {
//			Polyline* poly = *it;
//			laserPolylinePool.erase(it);
//			laserPolylineSpares.push_back(poly);
//			return true;
//		}
//
//	}
	
	protected:

	private:

};

}
