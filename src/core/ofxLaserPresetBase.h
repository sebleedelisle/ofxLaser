//
//  PresetBase.hpp
//  GenericTest
//
//  Created by Seb Lee-Delisle on 17/06/2022.
//

#pragma once

#include "ofMain.h"

namespace ofxLaser {
class PresetBase {
    
    public :
    PresetBase();
    
    virtual void serialize(ofJson&json){};
    virtual bool deserialize(ofJson&jsonGroup){};
    virtual const string& getLabel();
    virtual void setLabel(string _label);
    
    protected : 
    ofParameter<string> label;
    ofParameter<string> description;
   
    
};
}
