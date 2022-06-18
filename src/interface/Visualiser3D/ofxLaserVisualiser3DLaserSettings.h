//
//  ofxLaserVisualiser3DPreset.h
//
//  Created by Seb Lee-Delisle on 18/06/2022.
//

#pragma once
#include "ofxLaserPresetBase.h"
#include "ofxLaserLaser3DVisualObject.h"

namespace ofxLaser {
class Visualiser3DLaserSettings : public PresetBase {
    
    public :
    Visualiser3DLaserSettings();
    
    static string getFolderPath(){
        return "ofxLaser/visualiserlaserpresets";
    };
    static string getTypeName() {
        return "Visualiser 3D Laser";
    }
    Visualiser3DLaserSettings& operator=( Visualiser3DLaserSettings& that);
    bool operator == (Visualiser3DLaserSettings& that);
    bool operator != (Visualiser3DLaserSettings& that);
  
    virtual void serialize(ofJson&json) override;
    virtual bool deserialize(ofJson&jsonGroup) override;


    vector<Laser3DVisualObject> laserObjects;
    
};
}
