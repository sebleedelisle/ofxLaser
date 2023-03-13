//
//  ofxLaserVisualiser3D.cpp
//
//  Created by Seb Lee-Delisle on 10/06/2022.
//


#include "ofxLaserVisualiser3D.h"

using namespace ofxLaser;


Visualiser3D :: Visualiser3D() {
    
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
    
   
    load();
    ofAddListener(params.parameterChangedE(), this, &Visualiser3D::paramsChanged);
    ofAddListener(settings.params.parameterChangedE(), this, &Visualiser3D::paramsChanged);
   
//    ofAddListener(ofEvents().mousePressed, this, &Visualiser3D::mousePressed, OF_EVENT_ORDER_BEFORE_APP);
//    ofAddListener(ofEvents().mouseMoved, this, &Visualiser3D::mouseMoved, OF_EVENT_ORDER_BEFORE_APP);
//    ofAddListener(ofEvents().mouseReleased, this, &Visualiser3D::mouseReleased, OF_EVENT_ORDER_BEFORE_APP);
//    ofAddListener(ofEvents().mouseDragged, this, &Visualiser3D::mouseDragged, OF_EVENT_ORDER_BEFORE_APP);
    ofAddListener(ofEvents().mouseScrolled, this, &Visualiser3D::mouseScrolled, OF_EVENT_ORDER_BEFORE_APP);

    // TODO - listener for laser settings - how? 

}
Visualiser3D :: ~Visualiser3D() {
    
    ofRemoveListener(settings.params.parameterChangedE(), this, &Visualiser3D::paramsChanged);
//    
//    ofRemoveListener(ofEvents().mousePressed, this, &Visualiser3D::mousePressed, OF_EVENT_ORDER_APP);
//    ofRemoveListener(ofEvents().mouseMoved, this, &Visualiser3D::mouseMoved, OF_EVENT_ORDER_APP);
//    ofRemoveListener(ofEvents().mouseReleased, this, &Visualiser3D::mouseReleased, OF_EVENT_ORDER_APP);
//    ofRemoveListener(ofEvents().mouseDragged, this, &Visualiser3D::mouseDragged, OF_EVENT_ORDER_APP);
    ofRemoveListener(ofEvents().mouseScrolled, this, &Visualiser3D::mouseScrolled, OF_EVENT_ORDER_APP);

}


bool Visualiser3D :: mousePressed(ofMouseEventArgs &e) {
    //ofLogNotice("Visualiser3D :: mousePressed");
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
    return false;
}
bool Visualiser3D :: mouseReleased(ofMouseEventArgs &e) {
    dragging = false;
    return false;
}

bool Visualiser3D :: mouseDragged(ofMouseEventArgs &e) {
    
    if(dragging) {
        glm::vec2 v = (e-lastMousePosition);
        lastMousePosition = e;
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
        
    }
    return false;
}

void Visualiser3D :: update() {
    
   
    //update view stuff
    
    if(smoothedCameraOrbit != settings.cameraOrbit.get()) {
        smoothedCameraOrbit += ((settings.cameraOrbit.get()-smoothedCameraOrbit)*0.4);
        gridDirty = true; 

    }
    
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

void Visualiser3D :: draw(const ofRectangle& rect, const vector<Laser*>& lasers, bool isdragactive) {
    
    dragActive = isdragactive;
    
    update();
    numLasers = lasers.size();
    
    fboRect = rect;
        
    while(lasers.size()>lasersettings.laserObjects.size()) {
        lasersettings.laserObjects.emplace_back();
        
    }
    
    if((!visFbo.isAllocated()) || (visFbo.getWidth()!=rect.getWidth()) || (visFbo.getHeight()!=rect.getHeight())) {
        visFbo.allocate(rect.getWidth(), rect.getHeight(), GL_RGBA, 4);
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
    float groundLevel = 10;
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
            
            
            grid.clear();
            grid.setMode(OF_PRIMITIVE_LINES);
            for(int x = -1000; x<1000; x+=gridspacing) {
                for(int z = -1000; z<1000; z+=gridspacing) {
                    
                    ofColor col = ofColor(0,50,0);
                    ofPoint p (x, groundLevel, z);
                    ofPoint v = camera.getGlobalPosition();
                    float dist = v.distance(p);
                    float brightness = ofMap(dist, 500,1000,1,0,true);
                    if(brightness>0) {
                        col*=brightness;
                        grid.addVertex(glm::vec3(x, groundLevel, z));
                        grid.addVertex(glm::vec3(x, groundLevel, z+gridspacing));
                        grid.addColor(col);
                        grid.addColor(col);
                        
                        grid.addVertex(glm::vec3(x, groundLevel, z));
                        grid.addVertex(glm::vec3(x+gridspacing,groundLevel,z));
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
//
//    ofDeserialize(json, settings.params);
//    ofJson& laser3DsJson = json["laser3Ds"];
//
//    while(settings.laserObjects.size()<laser3DsJson.size())
//        settings.laserObjects.emplace_back();
//
//    for(int i = 0; i<settings.laserObjects.size(); i++) {
//        ofDeserialize(laser3DsJson[i], settings.laserObjects[i].visual3DParams);
//    }
    
}
void Visualiser3D ::save() {
    
    ofJson json;
    ofSerialize(json, params);
    ofSavePrettyJson("ofxLaser/Visualiser3D.json", json);
    
    ofJson json1;
    settings.serialize(json1);
    ofSavePrettyJson("ofxLaser/Visualiser3DSettings.json",json1);
    
    ofJson json2;
    lasersettings.serialize(json2);
    ofSavePrettyJson("ofxLaser/Visualiser3DLasers.json",json2);
    
    //ofLogNotice("Visualiser3D json : " ) << json.dump(3);
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
