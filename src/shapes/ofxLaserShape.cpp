//
//  LaserShape.h
//
//  Created by Seb Lee-Delisle on 20/05/2021.
//
//

#include "ofxLaserShape.h"

using namespace ofxLaser;

vector<float>& Shape :: getPointsAlongDistance(float distance, float acceleration, float speed, float speedMultiplier){
    
    speed*=speedMultiplier;
    acceleration*=speedMultiplier;
    unitDistances.clear();
    
    // otherwise, we'll get NaNs! 
    if(distance == 0) {
        unitDistances.push_back(0);
        return unitDistances;
        
    }
    
    float acceleratedistance = (speed*speed) / (2*acceleration);
    float timetogettospeed = speed / acceleration;
    
    float totaldistance = distance;
    
    float constantspeeddistance = totaldistance - (acceleratedistance*2);
    float constantspeedtime = constantspeeddistance/speed;
    
    if(totaldistance<(acceleratedistance*2)) {
        
        constantspeeddistance = 0 ;
        constantspeedtime = 0;
        acceleratedistance = totaldistance/2;
        speed = sqrt( acceleratedistance * 2 * acceleration);
        timetogettospeed = speed / acceleration;
        
    }
    
    float totaltime = (timetogettospeed*2) + constantspeedtime;
    
    float timeincrement = totaltime / (floor(totaltime));
    
    float currentdistance;
    
    float t = 0;
    
    while (t <= totaltime + 0.001) {
        
        if(t>totaltime) t = totaltime;
        
        if(t <=timetogettospeed) {
            currentdistance = 0.5 * acceleration * (t*t);
            
        } else if((t>timetogettospeed) && (t<=timetogettospeed+constantspeedtime)){
            currentdistance = acceleratedistance + ((t-timetogettospeed) * speed);
            
        } else  {
            float t3 = t - (timetogettospeed + constantspeedtime);
            
            currentdistance = (acceleratedistance + constantspeeddistance) + (speed*t3)+(0.5 *(-acceleration) * (t3*t3));
            
            
        }
        
        unitDistances.push_back(currentdistance/totaldistance);
        
        t+=timeincrement;
        
    }
    
    return unitDistances;
    
}


ofPoint& Shape :: getStartPos(){
    if(reversed && reversable) return endPos;
    else return startPos;
}
ofPoint& Shape :: getEndPos(){
    if(reversed && reversable) return startPos;
    else return endPos;
};

ofFloatColor& Shape :: getColour() {
    return colour;
}
void Shape :: appendPointsToVector(vector<ofxLaser::Point>& points, const RenderProfile& profile, float speedMultiplier) {
    
};
//
//void Shape :: setTargetZone(int zonenumber) {
//    targetZoneNumber = zonenumber;
//}
//
//int Shape :: getTargetZone() {
//    return targetZoneNumber;
//}
