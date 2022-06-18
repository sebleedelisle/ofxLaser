//
//  PresetBase.hpp
//  GenericTest
//
//  Created by Seb Lee-Delisle on 17/06/2022.
//

#pragma once

#include "ofMain.h"

class PresetBase {
    
    public :
    PresetBase();
    
//    void serialize(ofJson&json);
//    bool deserialize(ofJson&jsonGroup);
    const string& getLabel();
    void setLabel(string _label);
    
    ofParameter<string> label;
    ofParameter<string> description;
   
    
};
