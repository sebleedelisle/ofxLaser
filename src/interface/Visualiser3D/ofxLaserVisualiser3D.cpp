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
    load();
    
    if(visualiserPresetManager.getPreset("Default")==nullptr) {
        visualiserPresetManager.addPreset(settings);
    }
    if(visualiserLaserPresetManager.getPreset("Default")==nullptr) {
        visualiserLaserPresetManager.addPreset(lasersettings);
    }
    ofAddListener(settings.params.parameterChangedE(), this, &Visualiser3D::paramsChanged);

    // TODO - listener for laser settings - how? 

}
Visualiser3D :: ~Visualiser3D() {
    ofRemoveListener(settings.params.parameterChangedE(), this, &Visualiser3D::paramsChanged);
}



void Visualiser3D :: update() {
    //update view stuff
    
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

void Visualiser3D :: draw(const ofRectangle& rect, const vector<Laser*>& lasers) {
    
    update();
    numLasers = lasers.size();
    
    
    while(lasers.size()>lasersettings.laserObjects.size()) {
        lasersettings.laserObjects.emplace_back();
        
    }
    
    if((!visFbo.isAllocated()) || (visFbo.getWidth()!=rect.getWidth()) || (visFbo.getHeight()!=rect.getHeight())) {
        visFbo.allocate(rect.getWidth(), rect.getHeight(), GL_RGBA, 4);
    }
    
    visFbo.begin();
    
    ofBackground(0);
    
    // ofSetDepthTest(true);
    ofSetupScreenPerspective(800, 350, settings.cameraFov,0.1,100000);
    
    float eyeY = 800 / 2;
    float halfFov = PI * settings.cameraFov / 360;
    float theTan = tanf(halfFov);
    float dist = eyeY / theTan;
    //ofLogNotice()<<dist;
    
    ofPushMatrix();
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    
    //float scalefactor = (float)rect.getWidth()/800.0f;
   // ofScale(scalefactor);
    ofTranslate(400,175,600);
    
    
    ofRotateYDeg(settings.cameraOrientation.get().x);
    ofRotateXDeg(settings.cameraOrientation.get().y);
   
    ofTranslate(0,0,-settings.cameraDistance); // pull back z units
    
    ofRotateXDeg(settings.cameraOrbit.get().y);
    ofRotateYDeg(settings.cameraOrbit.get().x);
   
    
    ofPoint mousepos(ofGetMouseX(), ofGetMouseY());
    
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
        ofRotateXDeg(laser3D.orientation.get().x);
        ofRotateYDeg(laser3D.orientation.get().y);
        ofRotateZDeg(laser3D.orientation.get().z);
        
        ofNoFill();
        ofSetLineWidth(1);
        if(i<lasers.size()) {
            ofSetColor(0,100,0);
        } else {
            ofSetColor(0,50,0);
        }
        ofDrawBox(0, 0, -5, 7, 5, 10);
        
        
        if(i<lasers.size()) {
            ofxLaser::Laser& laser = *lasers.at(i);
           
            //vector<ofxLaser::Point>& laserPoints = laser.getLaserPoints();
            vector<glm::vec3>& points = laser.previewPathMesh.getVertices();
            vector<ofColor>& colours = laser.previewPathColours;

            float brightnessfactor = MAX(0.01f,5.0f/(float)points.size()) * settings.brightness;
            
            
            ofMesh laserMesh;
            
            for(int i = 1; i<points.size(); i++) {
                
                
                const glm::vec3& lp1 = points[i-1];
                const glm::vec3& lp2 = points[i];
                
                
                ofFloatColor colour1 = ofFloatColor(colours[i-1])*brightnessfactor; // lp1.getColour();
                ofFloatColor colour2 = ofFloatColor(colours[i])*brightnessfactor;//lp2.getColour()*brightnessfactor;
                
                // find 3 points that make a triangle
                // the point of the triangle is at the laser position
                // then the two corners are points in the laser path
                // Z is negative? into the screen
                // laser positions are relative to 0,0,0 which should be in the
                // distance in the centre of the screen.
                ofPoint p1, p2;
          
                p1.z = 250;
                p2.z = 250;
                p1.rotate(ofMap(lp1.x, 0, 800, -laser3D.horizontalRangeDegrees/2, laser3D.horizontalRangeDegrees/2), ofPoint(0,1,0));
                p2.rotate(ofMap(lp2.x, 0, 800, -laser3D.horizontalRangeDegrees/2, laser3D.horizontalRangeDegrees/2), ofPoint(0,1,0));
                p1.rotate(ofMap(lp1.y, 0, 800, -laser3D.verticalRangeDegrees/2, laser3D.horizontalRangeDegrees/2), ofPoint(-1,0,0));
                p2.rotate(ofMap(lp2.y, 0, 800, -laser3D.verticalRangeDegrees/2, laser3D.horizontalRangeDegrees/2), ofPoint(-1,0,0));
                
                //glm::vec3 beamnormal = glm::normalize(p1-laserpos);
                
                
                
                laserMesh.addVertex(ofPoint(0,0,0));
                laserMesh.addColor(colour1);
                laserMesh.addVertex(p1);
                laserMesh.addColor(colour1*0.1f);
                laserMesh.addVertex(p2);
                laserMesh.addColor(colour2*0.0f);
                
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
    visFbo.end();
    visFbo.draw(rect.getTopLeft());
    
    
    
    
}

void Visualiser3D :: drawGrid() {
    
    if(true) { // grid.size()==0)) {
        grid.clear();
        grid.setMode(OF_PRIMITIVE_LINES);
        for(int x = -500; x<500; x+=10) {
            grid.addVertex(glm::vec3(x, 40, -200));
            grid.addVertex(glm::vec3(x, 40, 200));
            grid.addColor(ofColor(0, 0, 0));
            grid.addColor(ofColor(0, 50, 0));
        }
        for(int z = -300; z<100; z+=10) {
            grid.addVertex(glm::vec3(-500, 40, z));
            grid.addVertex(glm::vec3(500, 40, z));
            grid.addColor(ofColor(0, ofMap(z,-300,100,0,50), 0));
            grid.addColor(ofColor(0, ofMap(z,-300,100,0,50), 0));
        }

    }
    grid.draw();
    
}


void Visualiser3D ::load() {
    ofJson json = ofLoadJson("ofxLaser/visualiser3D.json");
    settings.deserialize(json);
    
    
    ofJson json2 = ofLoadJson("ofxLaser/visualiser3DLasers.json");
    lasersettings.deserialize(json2); 
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

    settings.serialize(json);
    

    ofSavePrettyJson("ofxLaser/Visualiser3D.json",json);
    
    ofJson json2;
    lasersettings.serialize(json2);
    ofSavePrettyJson("ofxLaser/Visualiser3DLasers.json",json2);
    
    //ofLogNotice("Visualiser3D json : " ) << json.dump(3);
    
}


void Visualiser3D ::drawUI(){
    
    UI::startWindow("3D Visualiser", ImVec2(100,100), ImVec2(500,0));

    visualiserPresetManager.drawComboBox(settings);
    visualiserPresetManager.drawSaveButtons(settings);
    
    Visualiser3DSettings& currentPreset = *visualiserPresetManager.getPreset(settings.getLabel());
    
    UI::addResettableFloatSlider(settings.brightness, currentPreset.brightness); //.set("Camera FOV",
    UI::addResettableFloatSlider(settings.cameraDistance, currentPreset.brightness); //.set("Camera FOV", 45,10,120))
    UI::addFloatSlider(settings.cameraFov); //.set("Camera FOV", 45,10,120));
    
    UI::addFloat3Slider(settings.cameraOrbit);//.set("Camera position", glm::vec3(0,-2000,0)));
    UI::addFloat3Slider(settings.cameraOrientation);//.set("Camera orientation", glm::vec3(0,0,0)));
    
    ImGui::Separator();
    
    visualiserLaserPresetManager.drawComboBox(lasersettings);
    visualiserLaserPresetManager.drawSaveButtons(lasersettings);
    
    for(int i = 0; i<lasersettings.laserObjects.size(); i++) {
        ImGui::Separator();
        ImGui::Text("Laser %d", i+1);
        Laser3DVisualObject& laser3D = lasersettings.laserObjects[i];
        UI::addFloat3Slider(laser3D.position, "%.0f", 1, "Position##"+ofToString(i));
        UI::addFloat3Slider(laser3D.orientation, "%.0f", 1, "Orientation##"+ofToString(i));
    }
    
    if(numLasers<lasersettings.laserObjects.size()) {
        if(UI::Button("Remove extra 3D laser objects")){
            lasersettings.laserObjects.resize(numLasers);
            
        }
        
    }
    
    
    UI::endWindow();
}
