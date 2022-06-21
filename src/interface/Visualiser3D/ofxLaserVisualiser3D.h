//
//  ofxLaserVisualiser3D.h
//  example_LaserEffects
//
//  Created by Seb Lee-Delisle on 10/06/2022.
//

#pragma once
#include "ofxLaserLaser.h"
#include "ofxLaserLaser3DVisualObject.h"
#include "ofxLaserPresetManager.h"
#include "ofxLaserVisualiser3DSettings.h"
#include "ofxLaserVisualiser3DLaserSettings.h"

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
    ofCamera camera; 
    //vector<Laser3DVisualObject*> laser3Ds;
    
    PresetManager<Visualiser3DSettings> visualiserPresetManager;
    Visualiser3DSettings settings;
    PresetManager<Visualiser3DLaserSettings> visualiserLaserPresetManager;
    Visualiser3DLaserSettings lasersettings;
   
    //ofParameterGroup params;
     
    
    ofMesh grid; 
    bool dirty = false; 
    
};
}

