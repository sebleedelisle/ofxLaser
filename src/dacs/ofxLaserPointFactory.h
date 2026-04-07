//
//  ofxLaserPointFactory.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 15/01/2019.
//
//

#pragma once
#include "ofMain.h"
#include "ofxLaserPoint.h"
#include <vector>
#include <mutex>

namespace ofxLaser {
class PointFactory {

	public:

	static void releasePoint(Point* pointToRelease) {
		std::lock_guard<std::mutex> lock(poolMutex);
		pool.push_back(pointToRelease);
	}
	static Point* getPoint(const Point& pointToClone) {
		return PointFactory::getPoint(&pointToClone);
	}
	static Point* getPoint(const Point* pointToClone) {
		Point* point = borrowObject();
        *point = *pointToClone;
		return point;
	}
    static Point* getPoint(bool clear = true) {
        Point* point = borrowObject();
        return point;
    }


	protected:
	static Point* borrowObject() {
		std::lock_guard<std::mutex> lock(poolMutex);
		if(!pool.empty()) {
			Point* obj = pool.back();
			pool.pop_back();
			return obj;
		}
		return new Point();
	}

	static std::vector<Point*> pool;
	static std::mutex poolMutex;

	private:

};

}
