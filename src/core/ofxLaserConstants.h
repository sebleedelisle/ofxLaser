//
//  constants.h
//  ofxLaser
//
//  Created by Seb Lee-Delisle on 20/04/2021.
//

#pragma once
#include <string>
#include <vector>

#define OFXLASER_PROFILE_FAST "FAST"
#define OFXLASER_PROFILE_DEFAULT "DEFAULT"
#define OFXLASER_PROFILE_DETAIL "DETAIL"

const std::vector<std::string> OFXLASER_RENDER_PROFILES = {OFXLASER_PROFILE_DEFAULT, OFXLASER_PROFILE_DETAIL, OFXLASER_PROFILE_FAST };
const std::vector<std::string> OFXLASER_RENDER_LABELS = {"Default", "Detail", "Fast" };
