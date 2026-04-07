//
//  ofxLaserFactory.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 15/01/2019.
//
//

#pragma once
#include "ofMain.h"
#include <vector>
#include <mutex>

namespace ofxLaser {
class Factory {

	public:

	static void releasePolyline(ofPolyline* polyToRelease) {
		std::lock_guard<std::mutex> lock(poolMutex);
		pool.push_back(polyToRelease);
	}
	static ofPolyline* getPolyline(const ofPolyline& polyToClone) {
		return Factory::getPolyline(&polyToClone);
	}
	static ofPolyline* getPolyline(const ofPolyline* polyToClone) {
		ofPolyline* poly = borrowObject();
        *poly = *polyToClone;
		return poly;
	}
    static ofPolyline* getPolyline(bool clear = true) {
        ofPolyline* poly = borrowObject();
        if(clear) poly->clear();
        return poly;
    }


	protected:
	static ofPolyline* borrowObject() {
		std::lock_guard<std::mutex> lock(poolMutex);
		if(!pool.empty()) {
			ofPolyline* obj = pool.back();
			pool.pop_back();
			return obj;
		}
		return new ofPolyline();
	}

	static std::vector<ofPolyline*> pool;
	static std::mutex poolMutex;

	private:

};

}
