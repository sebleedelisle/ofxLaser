//
//  ofxLaserScannerSettings.cpp
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 20/04/2021.
//

#include "ofxLaserScannerSettings.h"

using namespace ofxLaser;

ScannerSettings :: ScannerSettings() {
    params.add(moveSpeed.set("Move Speed", 4.0 ,0.1,50));
    params.add(shapePreBlank.set("Hold off before", 1,0,8));
    params.add(shapePreOn.set("Hold on before", 0,0,8));
    params.add(shapePostOn.set("Hold on after", 0,0,8));
    params.add(shapePostBlank.set("Hold off after", 1,0,8));

    // second argument is passed into constructor for the RenderProfile
    renderProfiles.emplace(OFXLASER_PROFILE_FAST, ofToString("Fast"));
    renderProfiles.emplace(OFXLASER_PROFILE_DEFAULT, ofToString("Default"));
    renderProfiles.emplace(OFXLASER_PROFILE_DETAIL, ofToString("High detail"));

    RenderProfile& fast = renderProfiles.at(OFXLASER_PROFILE_FAST);
    RenderProfile& defaultProfile = renderProfiles.at(OFXLASER_PROFILE_DEFAULT);
    RenderProfile& detail = renderProfiles.at(OFXLASER_PROFILE_DETAIL);

    renderParams.add(defaultProfile.params);
    renderParams.add(fast.params);
    renderParams.add(detail.params);


    defaultProfile.speed = 4;
    defaultProfile.acceleration = 0.5;
    defaultProfile.cornerThreshold  = 40;
    defaultProfile.dotMaxPoints = 10;

    fast.speed = 6;
    fast.acceleration = 1.2;
    fast.cornerThreshold  = 80;
    fast.dotMaxPoints = 5;

    detail.speed = 2.5;
    detail.acceleration = 0.75;
    detail.cornerThreshold  = 15;
    detail.dotMaxPoints = 20;
    
    params.add(renderParams);

}
