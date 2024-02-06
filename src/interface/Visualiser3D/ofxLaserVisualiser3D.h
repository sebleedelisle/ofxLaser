//
//  ofxLaserVisualiser3D.h
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
        gridDirty = true;
    }
    
    bool mouseMoved(ofMouseEventArgs &e);
    bool mousePressed(ofMouseEventArgs &e);
    bool mouseReleased(ofMouseEventArgs &e);
    bool mouseDragged(ofMouseEventArgs &e);
    bool mouseScrolled(ofMouseEventArgs &e);
    
    glm::vec2 lastMousePosition;
    bool dragging = false;
    bool changeTarget = false;
    
    void load();
    void save();
    void serialize(ofJson& json);
    bool deserialize(ofJson& json);
    void update();
    void draw(const ofRectangle& rect, const vector<Laser*>& lasers, ofFbo& canvasFbo, bool isdragactive);
    void drawGrid();
    
    void drawUI(); 
    
    int numLasers;
    
    ofFbo visFbo;
    ofRectangle fboRect;
    ofCamera camera;
    glm::vec2 smoothedCameraOrbit;
    glm::vec3 smoothedCameraTarget;
    //vector<Laser3DVisualObject*> laser3Ds;
    
    PresetManager<Visualiser3DSettings> visualiserPresetManager;
    Visualiser3DSettings settings;
    PresetManager<Visualiser3DLaserSettings> visualiserLaserPresetManager;
    Visualiser3DLaserSettings lasersettings;
   
    ofParameterGroup params;
    ofParameter<bool> showLaserNumbers;
    ofParameter<bool> showZoneNumbers;
    ofParameter<float> brightness;
    bool showSettingsWindow; 

    bool showCanvas;
    glm::vec3 canvasPos;
    glm::vec3 canvasRotation;
    glm::vec3 canvasScale;
    
    
    ofMesh grid;
    float gridHeight; 
    bool dirty = false; 
    bool gridDirty = true;
    bool dragActive = false;
};
}

