//
//  ofxLaserVisualiser3DPreset.h
//
//  Created by Seb Lee-Delisle on 18/06/2022.
//

#pragma once
#include "ofxLaserPresetBase.h"
#include "ofxLaserLaser3DVisualObject.h"

namespace ofxLaser {
class Visualiser3DSettings : public PresetBase {
    
    public :
    Visualiser3DSettings();
    
    static string getFolderPath(){
        return "ofxLaser/visualiserpresets";
    };
    
    Visualiser3DSettings& operator=( Visualiser3DSettings& that);
    bool operator == (Visualiser3DSettings& that);
    bool operator != (Visualiser3DSettings& that);
  
    virtual void serialize(ofJson&json) override;
    virtual bool deserialize(ofJson&jsonGroup) override;

    ofParameter<glm::vec3> cameraOrbit;
    ofParameter<glm::vec3> cameraOrientation;
    ofParameter<float> cameraDistance;
    ofParameter<float> cameraFov;
    ofParameter<float> brightness;

    vector<Laser3DVisualObject> laserObjects;
    
};
}
