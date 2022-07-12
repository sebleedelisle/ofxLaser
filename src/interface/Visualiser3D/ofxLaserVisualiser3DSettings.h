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
    static string getTypeName() {
        return "Visualiser 3D";
    }
    Visualiser3DSettings& operator=( Visualiser3DSettings& that);
    bool operator == (Visualiser3DSettings& that);
    bool operator != (Visualiser3DSettings& that);
  
    virtual void serialize(ofJson&json) const override;
    virtual bool deserialize(ofJson&jsonGroup) override;

    ofParameter<glm::vec2> cameraOrbit;
    ofParameter<glm::vec3> cameraOrbitTarget;
    ofParameter<float> cameraDistance;
    ofParameter<float> cameraFov;


    

    
};
}
