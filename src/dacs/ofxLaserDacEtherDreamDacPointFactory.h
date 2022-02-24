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
#include "ofxLaserDacEtherDreamDacPoint.h"

namespace ofxLaser {
class EtherDreamDacPointFactory {
	
	public:
	
	static Poco::ObjectPool<EtherDreamDacPoint> pointObjectPool;
	
	
	static void releasePoint(EtherDreamDacPoint* pointToRelease) {

		pointObjectPool.returnObject( pointToRelease);
		
	}
	static EtherDreamDacPoint* getPoint(const EtherDreamDacPoint& pointToClone) {
		return EtherDreamDacPointFactory::getPoint(&pointToClone);
	}
	static EtherDreamDacPoint* getPoint(const EtherDreamDacPoint* pointToClone) {

        EtherDreamDacPoint* point;

        point = pointObjectPool.borrowObject();
		
        *point = *pointToClone;
	
		return point;
	}
    static EtherDreamDacPoint* getPoint(bool clear = true) {

        EtherDreamDacPoint* point;

        point = pointObjectPool.borrowObject();
        
        if(clear) point->clear();
        
        return point;
    }

	
	protected:

	private:

};

}
