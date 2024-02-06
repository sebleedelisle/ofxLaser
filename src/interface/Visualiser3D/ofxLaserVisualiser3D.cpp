//
//  ofxLaserVisualiser3D.cpp
//
//  Created by Seb Lee-Delisle on 10/06/2022.
//


#include "ofxLaserVisualiser3D.h"

using namespace ofxLaser;


Visualiser3D :: Visualiser3D() {
    showSettingsWindow = false; 
    lasersettings.setLabel("Default");
    settings.setLabel("Default");

    if(visualiserPresetManager.getPreset("Default")==nullptr) {
        visualiserPresetManager.addPreset(settings);
    }
    if(visualiserLaserPresetManager.getPreset("Default")==nullptr) {
        visualiserLaserPresetManager.addPreset(lasersettings);
    }
    params.setName("Visualiser3D"); 
    params.add(brightness.set("Brightness adjustment", 10,0.1,30));
    params.add(showLaserNumbers.set("Show laser numbers", false));
    params.add(showZoneNumbers.set("Show zone numbers", false));
    
    canvasPos = {0,-110,0};
    canvasScale = {0.5, 0.5,0.5};
    canvasRotation = {0, 0, 0};
    showCanvas = false;
    
    // TODO this needs to go somewhere else!
    load();
    ofAddListener(params.parameterChangedE(), this, &Visualiser3D::paramsChanged);
    ofAddListener(settings.params.parameterChangedE(), this, &Visualiser3D::paramsChanged);
   
//    ofAddListener(ofEvents().mousePressed, this, &Visualiser3D::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
//    ofAddListener(ofEvents().mouseMoved, this, &Visualiser3D::mouseMoved, OF_EVENT_ORDER_BEFORE_APP);
//    ofAddListener(ofEvents().mouseReleased, this, &Visualiser3D::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
//    ofAddListener(ofEvents().mouseDragged, this, &Visualiser3D::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
//    ofAddListener(ofEvents().mouseScrolled, this, &Visualiser3D::mouseScrolled, OF_EVENT_ORDER_BEFORE_APP);
    // TODO - listener for laser settings - how? 
    
    
   
    
}
Visualiser3D :: ~Visualiser3D() {
    
    ofRemoveListener(settings.params.parameterChangedE(), this, &Visualiser3D::paramsChanged);
//    
//    ofRemoveListener(ofEvents().mousePressed, this, &Visualiser3D::mousePressed, OF_EVENT_ORDER_APP);
//    ofRemoveListener(ofEvents().mouseMoved, this, &Visualiser3D::mouseMoved, OF_EVENT_ORDER_APP);
//    ofRemoveListener(ofEvents().mouseReleased, this, &Visualiser3D::mouseReleased, OF_EVENT_ORDER_APP);
//    ofRemoveListener(ofEvents().mouseDragged, this, &Visualiser3D::mouseDragged, OF_EVENT_ORDER_APP);
 //   ofRemoveListener(ofEvents().mouseScrolled, this, &Visualiser3D::mouseScrolled, OF_EVENT_ORDER_APP);

}


bool Visualiser3D :: mousePressed(ofMouseEventArgs &e) {
    //ofLogNotice("Visualiser3D :: mousePressed");
    
    dragging = false;
    
    if(!fboRect.inside(e)) return false;
    if(!dragActive) return false;
    
    lastMousePosition = e;
    dragging = true;
    if(ofGetKeyPressed(OF_KEY_SHIFT)) {
        changeTarget = true;
    } else {
        changeTarget = false;
    }
    return false;
}

bool Visualiser3D :: mouseScrolled(ofMouseEventArgs &e) {

    if(!fboRect.inside(e)) return false;
    if(!dragActive) return false;
    
    if(changeTarget) {
        
    } else {
        settings.cameraDistance += e.scrollY*2;
    }
    return false;
}

bool Visualiser3D :: mouseMoved(ofMouseEventArgs &e) {
    if(dragging) {
        dragging = false;
    }
    return false;
}
bool Visualiser3D :: mouseReleased(ofMouseEventArgs &e) {
    if(dragging) {
        dragging = false;
    }
    return false;
}

bool Visualiser3D :: mouseDragged(ofMouseEventArgs &e) {
    
    if(dragging) {
        glm::vec2 v = (e-lastMousePosition);
        lastMousePosition = e;
        // hack for re-focus bug
        if(glm::length(v)<100) {
            if(changeTarget) {
                v*=0.2;
                glm::vec3 target = settings.cameraOrbitTarget.get();
                target.x-=v.x;
                target.y-=v.y;
                settings.cameraOrbitTarget.set(target);
                
            } else {
                //v*=0.2;
                glm::vec2 orbit = settings.cameraOrbit.get();
                orbit.y -= v.x*0.2;
                orbit.x += v.y*0.05f;
                settings.cameraOrbit.set(orbit);
            }
        } else {
            ofLogNotice("over drag bug fixed!");
        } 
    }
    return false;
}

void Visualiser3D :: update() {
    
   
    //update view stuff
    
    if(smoothedCameraOrbit != settings.cameraOrbit.get()) {
        smoothedCameraOrbit += ((settings.cameraOrbit.get()-smoothedCameraOrbit)*0.4);
        
        if(glm::distance(smoothedCameraOrbit, settings.cameraOrbit.get()) < 0.0001){
            smoothedCameraOrbit =settings.cameraOrbit.get();
        }
            
       // ofLogNotice() << smoothedCameraOrbit << " " << settings.cameraOrbit.get();
        
        gridDirty = true; 

    }
    if(settings.gridHeight!=gridHeight) gridDirty = true;
    
    // dirty is set when a param changes

    bool needsSave = dirty;
    for(Laser3DVisualObject& laser3D : lasersettings.laserObjects) {
        if(laser3D.dirty) {
            laser3D.dirty = false;
            needsSave = true;
        }
    }
    if (needsSave) {
        save();
    }
    dirty = false;
        
}

void Visualiser3D :: draw(const ofRectangle& rect, const vector<Laser*>& lasers, ofFbo& canvasFbo, bool isdragactive) {
    
    
    
    dragActive = isdragactive;
    
    update();
    numLasers = lasers.size();
    
    fboRect = rect;
        
    while(lasers.size()>lasersettings.laserObjects.size()) {
        lasersettings.laserObjects.emplace_back();
        
    }
    
    if((!visFbo.isAllocated()) || (visFbo.getWidth()!=round(rect.getWidth())) || (visFbo.getHeight()!=round(rect.getHeight()))) {
        ofLogNotice("Visualiser3D :: draw  - fbo.allocated");
        visFbo.clear();
        visFbo.allocate(round(rect.getWidth()), round(rect.getHeight()), GL_RGBA, 4);
    }
    
    visFbo.begin();
    
    ofBackground(0);
    
    // ofSetDepthTest(true);
  
    //ofSetupScreenPerspective(800, 350, settings.cameraFov,0.1,100000);
    
    //float eyeY = 800 / 2;
    //float halfFov = PI * settings.cameraFov / 360;
    //float theTan = tanf(halfFov);
    //float dist = eyeY / theTan;
    //ofLogNotice()<<dist;
    
    camera.begin();
    //camera.setPosition(0, 0, 0);
    camera.setVFlip(true);
    camera.setFov(settings.cameraFov);
    camera.orbitDeg(smoothedCameraOrbit.y, smoothedCameraOrbit.x, settings.cameraDistance, settings.cameraOrbitTarget);
    
    ofPushMatrix();
    ofPushStyle();
    ofDisableAlphaBlending();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    
    //float scalefactor = (float)rect.getWidth()/800.0f;
   // ofScale(scalefactor);
    //ofTranslate(400,175,600);
    
//
//    ofRotateYDeg(settings.cameraOrientation.get().x);
//    ofRotateXDeg(settings.cameraOrientation.get().y);
//
//    ofTranslate(0,0,-settings.cameraDistance); // pull back z units
//
//    ofRotateXDeg(settings.cameraOrbit.get().y);
//    ofRotateYDeg(settings.cameraOrbit.get().x);
//
//
//    ofPoint mousepos(ofGetMouseX(), ofGetMouseY());
    
//    ofRectangle fboRect(0,0,visFbo.getWidth(), visFbo.getHeight());
//    if(fboRect.inside(mousepos)) {
//        ofTranslate(ofMap(mousepos.x, 0, fboRect.getWidth(), 30,-30), ofMap (mousepos.y, 0, fboRect.getWidth(), 100,-100) );
//    }
    
    
    drawGrid();
    
    if(showCanvas && canvasFbo.isAllocated()) {
        ofDisableBlendMode();
        
        ofPushMatrix();
        ofTranslate(canvasPos);
        ofScale(canvasScale);
        // todo - fix order
        ofRotateXDeg(canvasRotation.x);
        ofRotateYDeg(canvasRotation.y);
        ofRotateZDeg(canvasRotation.z);

        canvasFbo.setAnchorPercent(0.5,0.5);
        
        canvasFbo.draw(0,0);
        canvasFbo.setAnchorPercent(0,0);
        ofPopMatrix();
        ofEnableBlendMode(OF_BLENDMODE_ADD);
    }
    
    for(size_t i= 0; i<lasersettings.laserObjects.size(); i++) {
        
        Laser3DVisualObject& laser3D = lasersettings.laserObjects.at(i);
        
        ofPushStyle();
        ofPushMatrix();
        
        
        //move to the laser position
        ofTranslate(laser3D.position);
        if(i<lasers.size()) {
            ofSetColor(0,100,0);
        } else {
            ofSetColor(0,50,0);
        }
        
        if(showLaserNumbers) ofDrawBitmapString(ofToString(i+1), -2,-6);
        if((showZoneNumbers) && (i<lasers.size())) {
            ofxLaser::Laser* laser = lasers[i];
            vector<OutputZone*> outputzones = laser->getActiveZones();
            //vector<string> outputzonenumbers;
            int spacing = 3;
            int textwidth = 0;
            string zoneoutputstring;
            for(OutputZone* outputzone : outputzones) {
                if(outputzone->getIsAlternate()) continue;
                string label = outputzone->getLabel();
                zoneoutputstring += label+" ";
                //outputzonenumbers.push_back(label);
                textwidth+=(2+(label.size()*4));
                
            }
            ofDrawBitmapString(zoneoutputstring, -textwidth/2 ,10);
            
            
        }
        
        ofPushMatrix();
        ofRotateXDeg(-laser3D.orientation.get().x);
        ofRotateYDeg(laser3D.orientation.get().y);
        ofRotateZDeg(laser3D.orientation.get().z);
        
        ofNoFill();
        ofSetLineWidth(1);
        
        ofDrawBox(0, 0, -5, 7, 5, 10);
        
        ofPopMatrix();
        if(i<lasers.size()) {
            ofxLaser::Laser& laser = *lasers.at(i);
            bool flipX = laser3D.flipX;
            bool flipY = laser3D.flipY;
            //vector<ofxLaser::Point>& laserPoints = laser.getLaserPoints();
            vector<glm::vec3>& points = laser.previewPathMesh.getVertices();
            vector<ofFloatColor>& colours = laser.previewPathColoured.getColors();

            float brightnessfactor = MAX(0.01f,5.0f/(float)points.size()) * brightness;
            
            
            ofMesh laserMesh;
            
            for(int i = 1; i<points.size(); i++) {
                
                
                const glm::vec3& lp1 = points[i-1];
                const glm::vec3& lp2 = points[i];
               
                
                
                ofFloatColor colour1 = ofFloatColor(colours[i-1]); // lp1.getColour();
                ofFloatColor colour2 = ofFloatColor(colours[i]);//lp2.getColour()*brightnessfactor;
                
                // find 3 points that make a triangle
                // the point of the triangle is at the laser position
                // then the two corners are points in the laser path
                // Z is negative? into the screen
                // laser positions are relative to 0,0,0 which should be in the
                // distance in the centre of the screen.
                ofPoint p1, p2;
          
                p1.z = 1000;
                p2.z = 1000;
                p1.rotate(ofMap(lp1.y, flipY ? 800 : 0, flipY? 0 : 800, -laser3D.verticalRangeDegrees/2, laser3D.horizontalRangeDegrees/2)+ laser3D.orientation.get().x, ofPoint(-1,0,0));
                
                p2.rotate(ofMap(lp2.y, flipY ? 800 : 0, flipY? 0 : 800, -laser3D.verticalRangeDegrees/2, laser3D.horizontalRangeDegrees/2)+ laser3D.orientation.get().x, ofPoint(-1,0,0));
               
                p1.rotate(ofMap(lp1.x, flipX ? 800 : 0, flipX? 0 : 800, -laser3D.horizontalRangeDegrees/2, laser3D.horizontalRangeDegrees/2) + laser3D.orientation.get().y, ofPoint(0,1,0));
                p2.rotate(ofMap(lp2.x, flipX ? 800 : 0, flipX? 0 : 800, -laser3D.horizontalRangeDegrees/2, laser3D.horizontalRangeDegrees/2)+ laser3D.orientation.get().y, ofPoint(0,1,0));
                
                ofPoint beamNormal = p1.getNormalized();
                ofPoint cameraNormal = ofPoint(laser3D.position)-camera.getGlobalPosition();
                cameraNormal.normalize();
                float dotproduct = beamNormal.dot(cameraNormal);
                dotproduct = ofMap(abs(dotproduct),0.5,1,0,1,true);
                dotproduct=pow(dotproduct,20);
                //ofLogNotice()<<dotproduct;
                // if it's negative it's coming towards us, positive going away from us
                float directionBrightnessMultiplier = ofMap(abs(dotproduct), 0, 1,0.1,1);
                //ofLogNotice() << dotproduct << " " << " " << cameraNormal ;
                
                
                laserMesh.addVertex(ofPoint(0,0,0));
                laserMesh.addColor(colour1*directionBrightnessMultiplier*brightnessfactor);
                laserMesh.addVertex(p1);
                laserMesh.addColor(colour1*0.1f*directionBrightnessMultiplier*brightnessfactor);
                laserMesh.addVertex(p2);
                laserMesh.addColor(colour2*0.1f*directionBrightnessMultiplier*brightnessfactor);
                
                // laserMesh.draw();
                
            }
            
            laserMesh.setMode(OF_PRIMITIVE_TRIANGLES);
            laserMesh.draw();
            ofSetLineWidth(2);
            for(ofFloatColor& col : laserMesh.getColors()) col*=0.7;
            laserMesh.setMode(OF_PRIMITIVE_LINES);
            laserMesh.draw();
        }
        ofDisableBlendMode();
        ofPopMatrix();
        ofPopStyle();
    }
    
    
    ofPopStyle();
    ofPopMatrix();
    camera.end();
    visFbo.end();
    visFbo.draw(rect.getTopLeft());
    
}

void Visualiser3D :: drawGrid() {
    
   
    
    if(true) { // grid.size()==0)) {
    
//        ofRectangle visibleRectangle(camera.getGlobalPosition(), 0,0);
//        ofPoint leftEdgePoint(0,0,-1000);
//        ofPoint rightEdgePoint = leftEdgePoint;
//        leftEdgePoint.rotate(settings.cameraOrbit.get().y+ (settings.cameraFov/2), ofPoint(0,1,0));
//        rightEdgePoint.rotate( settings.cameraOrbit.get().y- (settings.cameraFov/2), ofPoint(0,1,0));
//        leftEdgePoint+=camera.getGlobalPosition();
//        rightEdgePoint+=camera.getGlobalPosition();
//        ofFill();
//        ofSetColor(255);
//        ofDrawSphere(leftEdgePoint,10);
//        ofDrawSphere(rightEdgePoint,10);
//        visibleRectangle.growToInclude(leftEdgePoint);
//        visibleRectangle.growToInclude(rightEdgePoint);
//
//        ofDrawRectangle(visibleRectangle);
        
        if(gridDirty) {
            gridDirty = false;
            float gridspacing = 20;
            gridHeight = settings.gridHeight;
            
            
            grid.clear();
            grid.setMode(OF_PRIMITIVE_LINES);
            for(int x = -1000; x<1000; x+=gridspacing) {
                for(int z = -1000; z<1000; z+=gridspacing) {
                    
                    ofColor col = ofColor(0,50,0);
                    ofPoint p (x, gridHeight, z);
                    ofPoint v = camera.getGlobalPosition();
                    float dist = v.distance(p);
                    float brightness = ofMap(dist, 500,1000,1,0,true);
                    if(brightness>0) {
                        col*=brightness;
                        grid.addVertex(glm::vec3(x, gridHeight, z));
                        grid.addVertex(glm::vec3(x, gridHeight, z+gridspacing));
                        grid.addColor(col);
                        grid.addColor(col);
                        
                        grid.addVertex(glm::vec3(x, gridHeight, z));
                        grid.addVertex(glm::vec3(x+gridspacing,gridHeight,z));
                        grid.addColor(col);
                        grid.addColor(col);
                    }
                }
            }

        }
    }
    grid.draw();
}

void Visualiser3D ::load() {
    string filename ="ofxLaser/Visualiser3D.json";
    if(!ofFile(filename).exists()) return;
    
    ofJson json = ofLoadJson(filename);

    ofDeserialize(json, params);
    
    if(json.contains("showcanvas")) {
        showCanvas =json["showcanvas"].get<bool>();
    }
    if(json.contains("canvaspos") && (json["canvaspos"].size()==3)) {
        canvasPos = {
            json["canvaspos"][0].get<float>(),
            json["canvaspos"][1].get<float>(),
            json["canvaspos"][2].get<float>()
        };
    }
    if(json.contains("canvasrotation") && (json["canvasrotation"].size()==3)) {
        canvasRotation= {
            json["canvasrotation"][0].get<float>(),
            json["canvasrotation"][1].get<float>(),
            json["canvasrotation"][2].get<float>()
        };
    }
    if(json.contains("canvasscale") && (json["canvasscale"].size()==3)) {
        canvasScale= {
            json["canvasscale"][0].get<float>(),
            json["canvasscale"][1].get<float>(),
            json["canvasscale"][2].get<float>()
        };
    }
             
    filename ="ofxLaser/Visualiser3DSettings.json";
    if(ofFile(filename).exists()) {
        ofJson json1 = ofLoadJson(filename);
        settings.deserialize(json1);
    }
    
    filename ="ofxLaser/visualiser3DLasers.json";
    if(ofFile(filename).exists()) {
        ofJson json2 = ofLoadJson(filename);
    
        lasersettings.deserialize(json2);
    }
    
    
    
}
void Visualiser3D ::save() {
    
    ofJson json;
    ofSerialize(json, params);
    
   
    json["showcanvas"] = showCanvas;
    json["canvaspos"] = {canvasPos.x, canvasPos.y, canvasPos.z};
    json["canvasrotation"] = {canvasRotation.x, canvasRotation.y, canvasRotation.z};
    json["canvasscale"] = {canvasScale.x, canvasScale.y, canvasScale.z};

    ofSavePrettyJson("ofxLaser/Visualiser3D.json", json);
    
    ofJson json1;
    settings.serialize(json1);
    ofSavePrettyJson("ofxLaser/Visualiser3DSettings.json",json1);
    
    ofJson json2;
    lasersettings.serialize(json2);
    ofSavePrettyJson("ofxLaser/Visualiser3DLasers.json",json2);
    
}


void Visualiser3D :: serialize(ofJson& json) {
    ofSerialize(json["params"], params);
    settings.serialize(json["settings"]);
    lasersettings.serialize(json["lasersettings"]);
    
}
bool Visualiser3D ::deserialize(ofJson& json) {
    bool success = true;
    
    ofDeserialize(json["params"], params);
    
    if(json.contains("settings")) {
        settings.deserialize(json["settings"]);
    } else {
        success = false;
    }
    if(json.contains("lasersettings")) {
        lasersettings.deserialize(json["lasersettings"]);
    } else {
        success = false;
    }
    return success;
}


void Visualiser3D ::drawUI(){
    
    if(!showSettingsWindow) return;
    
    if(UI::startWindow("3D Visualiser", ImVec2(100,100), ImVec2(500,0), 0, false, &showSettingsWindow)) {
        
        visualiserPresetManager.drawComboBox(settings);
        visualiserPresetManager.drawSaveButtons(settings);
        
        Visualiser3DSettings& currentPreset = *visualiserPresetManager.getPreset(settings.getLabel());
        
        UI::addResettableFloatSlider(brightness, 10);
        UI::addCheckbox(showLaserNumbers);
        UI::addCheckbox(showZoneNumbers);
        UI::addResettableFloatDrag(settings.cameraDistance, currentPreset.cameraDistance); //.set("Camera FOV", 45,10,120))
        UI::addResettableFloatDrag(settings.cameraFov, currentPreset.cameraFov); //.set("Camera FOV", 45,10,120));
        
        UI::addResettableFloat2Drag(settings.cameraOrbit, currentPreset.cameraOrbit, 1, "Orbits around the target point, pitch and yaw");//.set("Camera position", glm::vec3(0,-2000,0)));
        UI::addResettableFloat3Drag(settings.cameraOrbitTarget, currentPreset.cameraOrbitTarget, 1, "The point the camera is looking at");//.set("Camera orientation", glm::vec3(0,0,0)));
        
        UI::addResettableFloatDrag(settings.gridHeight, currentPreset.gridHeight); //.set("Camera
        
        ImGui::Separator();
        
      
        
        if(ImGui::Checkbox("Show canvas", &showCanvas)) {
            dirty = true;
        }
        if(UI::addFloat3Drag("Canvas position", canvasPos, 1, {-1000,-1000,-1000},  {1000,1000,1000} )) {
            dirty = true;
        }
        if(UI::addFloat3Drag("Canvas rotation", canvasRotation, 1, {-180,-180,-180},  {180,180,180} )) {
            dirty = true;
        }
        if(UI::addFloat3Drag("Canvas scale", canvasScale, 0.01, {0,0,0},  {5,5,5} )) {
            dirty = true;
        }
       
        
        ImGui::Separator();
        
        visualiserLaserPresetManager.drawComboBox(lasersettings);
        visualiserLaserPresetManager.drawSaveButtons(lasersettings);
        Visualiser3DLaserSettings& currentLaserPreset = *visualiserLaserPresetManager.getPreset(lasersettings.getLabel());
        
        for(int i = 0; i<lasersettings.laserObjects.size(); i++) {
            ImGui::Separator();
            string label = "Laser " + ofToString(i+1);
            if( ImGui::TreeNode(label.c_str())) {
                Laser3DVisualObject& laser3D = lasersettings.laserObjects[i];
                
                if(currentLaserPreset.laserObjects.size()>i) {
                    Laser3DVisualObject& laser3DCurrentPreset = currentLaserPreset.laserObjects[i];
                    UI::addResettableFloat3Drag(laser3D.position, laser3DCurrentPreset.position, 1, "", "%.0f", "##"+ofToString(i));
                    UI::addResettableFloat3Drag(laser3D.orientation,  laser3DCurrentPreset.orientation, 1, "", "%.0f", "##"+ofToString(i));
                    
                } else {
                    UI::addFloat3Drag(laser3D.position, 1, "%.0f", "##"+ofToString(i));
                    UI::addFloat3Drag(laser3D.orientation, 1, "%.0f", "##"+ofToString(i));
                }
                
                UI :: addCheckbox(laser3D.flipX);
                ImGui::SameLine();
                UI :: addCheckbox(laser3D.flipY);
                UI :: addParameter(laser3D.horizontalRangeDegrees);
                UI :: addParameter(laser3D.verticalRangeDegrees);
                ImGui::TreePop();
            }
            
        }
        
        if(numLasers<lasersettings.laserObjects.size()) {
            if(UI::Button("Remove extra 3D laser objects")){
                lasersettings.laserObjects.resize(numLasers);
                
            }
            
        }
        
        
        
    }
    
    UI::endWindow();
}
