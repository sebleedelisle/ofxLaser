//
//  ofxLaserColourSettings.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 20/04/2021.
//

#include "ofxLaserColourSettings.h"

using namespace ofxLaser;

ColourSettings :: ColourSettings(){
    
    params.add(label.set("label", "Default"));
    
    params.setName("Colour calibration");
    
    params.add(red100.set("red 100", 1,0,1));
    params.add(red75.set("red 75", 0.75,0,1));
    params.add(red50.set("red 50", 0.5,0,1));
    params.add(red25.set("red 25", 0.25,0,1));
    params.add(red0.set("red 0", 0,0,1));
    
    params.add(green100.set("green 100", 1,0,1));
    params.add(green75.set("green 75", 0.75,0,1));
    params.add(green50.set("green 50", 0.5,0,1));
    params.add(green25.set("green 25", 0.25,0,1));
    params.add(green0.set("green 0", 0,0,1));
    
    params.add(blue100.set("blue 100", 1,0,1));
    params.add(blue75.set("blue 75", 0.75,0,1));
    params.add(blue50.set("blue 50", 0.5,0,1));
    params.add(blue25.set("blue 25", 0.25,0,1));
    params.add(blue0.set("blue 0", 0,0,1));
    
    
};
float ColourSettings::calculateCalibratedBrightness(float value, float intensity, float level100, float level75, float level50, float level25, float level0){
    value/=255.0f;
    value *=intensity;
    if(value<0.001) {
        return 0;
    } else if(value<0.25) {
        return ofMap(value, 0, 0.25, level0, level25) *255;
    } else if(value<0.5) {
        return ofMap(value, 0.25, 0.5,level25, level50) *255;
    } else if(value<0.75) {
        return ofMap(value, 0.5, 0.75,level50, level75) *255;
    } else {
        return ofMap(value, 0.75, 1,level75, level100) *255;
    }
    
}

void ColourSettings::processColour(ofxLaser::Point& p, float brightness) {
    p.r = calculateCalibratedBrightness(p.r, brightness, red100, red75, red50, red25, red0);
    p.g = calculateCalibratedBrightness(p.g, brightness, green100, green75, green50, green25, green0);
    p.b = calculateCalibratedBrightness(p.b, brightness, blue100, blue75, blue50, blue25, blue0);
}


ColourSettings& ColourSettings::operator=( ColourSettings& that) {
    ofJson json;
    that.serialize(json);
    deserialize(json);
    return *this;
}
bool ColourSettings::operator == (ColourSettings& that){
    ofJson json1;
    that.serialize(json1);
    json1["label"] = "";
    //string jsonstring = json1.dump();
    ofJson json2;
    serialize(json2);
    json2["label"] = "";
    
    return json1.dump() == json2.dump();
}
bool ColourSettings::operator != (ColourSettings& that){
    return !(*this==that);
}
