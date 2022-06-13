//
//  ofxLaserVisualiser3D.h
//  example_LaserEffects
//
//  Created by Seb Lee-Delisle on 10/06/2022.
//

#pragma once
#include "ofxLaserLaser.h"
#include "ofxLaserLaser3DVisualObject.h"

namespace ofxLaser {

class Visualiser3D {
    
    public :
    
    Visualiser3D();
    ~Visualiser3D();
    
    void paramsChanged(ofAbstractParameter& e){
        dirty = true;
    }
    
    void load();
    void save();
    
    void update(); 
    void draw(const ofRectangle& rect, const vector<Laser*>& lasers);
    void drawGrid();
    
    void drawUI(); 
    
    int numLasers;
    
    ofFbo visFbo;
    vector<Laser3DVisualObject*> laser3Ds;
    ofParameter<glm::vec3> cameraOrbit;
    ofParameter<glm::vec3> cameraOrientation;
    ofParameter<float> cameraDistance;
    ofParameter<float> cameraFov;
    ofParameter<float> brightness; 
    
    ofParameterGroup params;
    
    
    ofMesh grid; 
    bool dirty = false; 
    
};
}

