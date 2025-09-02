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
#include "ofxLaserPoint.h"

namespace ofxLaser {
class PointFactory {
	
	public:
	
	static Poco::ObjectPool<Point> pointObjectPool;
	
	
	static void releasePoint(Point* pointToRelease) {

		pointObjectPool.returnObject( pointToRelease);
		
	}
	static Point* getPoint(const Point& pointToClone) {
		return PointFactory::getPoint(&pointToClone);
	}
	static Point* getPoint(const Point* pointToClone) {

       // ofLogNotice("PointFactory :: getPoint : ") << pointObjectPool.available();
        Point* point;

        point = pointObjectPool.borrowObject();
        try {
            *point = *pointToClone;
        } catch(...) {
            ofLogNotice() << pointObjectPool.available();
            throw;
        }
		return point;
	}
    static Point* getPoint(bool clear = true) {

        Point* point;

        point = pointObjectPool.borrowObject();
        
        //if(clear) point->clear();
        
        return point;
    }

	
	protected:

	private:

};

}
