//
//  ofxLaserScannerSettings.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 20/04/2021.
//

#include "ofxLaserScannerSettings.h"

using namespace ofxLaser;

ScannerSettings :: ScannerSettings() {
    
    //ofLogNotice("ScannerSettings default constructor called");

    params.setName("Scanner settings");
    params.add(label.set("label", "Default"));
    params.add(description.set("description", ""));
    params.add(moveSpeed.set("Move Speed", 4.0 ,0.1,50));
    params.add(shapePreBlank.set("Hold off before", 1,0,8));
    params.add(shapePreOn.set("Hold on before", 0,0,8));
    params.add(shapePostOn.set("Hold on after", 0,0,8));
    params.add(shapePostBlank.set("Hold off after", 1,0,8));

    // second argument is passed into constructor for the RenderProfile
    profileFast.setLabel("Fast");
    profileDefault.setLabel("Default");
    profileDetail.setLabel("High detail");
   
    renderParams.setName("Render profiles"); 
    renderProfiles.emplace(OFXLASER_PROFILE_FAST, profileFast);
    renderProfiles.emplace(OFXLASER_PROFILE_DEFAULT, profileDefault);
    renderProfiles.emplace(OFXLASER_PROFILE_DETAIL, profileDetail);

    renderParams.add(profileDefault.params);
    renderParams.add(profileFast.params);
    renderParams.add(profileDetail.params);

    profileDefault.speed = 4;
    profileDefault.acceleration = 0.5;
    profileDefault.cornerThreshold  = 30;
    profileDefault.dotMaxPoints = 10;

    profileFast.speed = 6;
    profileFast.acceleration = 1.2;
    profileFast.cornerThreshold  = 80;
    profileFast.dotMaxPoints = 5;

    profileDetail.speed = 2.5;
    profileDetail.acceleration = 0.75;
    profileDetail.cornerThreshold  = 15;
    profileDetail.dotMaxPoints = 20;
    
    params.add(renderParams);

}


ScannerSettings& ScannerSettings::operator=( ScannerSettings& that) {
    ofJson json;
    that.serialize(json);
    deserialize(json);
    return *this;
}
bool ScannerSettings::operator == (ScannerSettings& that){
    ofJson json1;
    that.serialize(json1);
    json1["label"] = "";
    //string jsonstring = json1.dump();
    ofJson json2;
    serialize(json2);
    json2["label"] = "";
    
    return json1.dump() == json2.dump();
}
bool ScannerSettings::operator != (ScannerSettings& that){
    return !(*this==that); 
}
