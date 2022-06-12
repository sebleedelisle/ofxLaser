//
//  ofxLaserVisualiser3D.cpp
//
//  Created by Seb Lee-Delisle on 10/06/2022.
//


#include "ofxLaserVisualiser3D.h"

using namespace ofxLaser;


Visualiser3D :: Visualiser3D() {
    params.add(cameraDistance.set("Camera distance", 2000,500,5000));
    params.add(cameraFov.set("Camera FOV", 45,10,120));
    
    params.add(cameraOrbit.set("Camera orbit", glm::vec3(0,0,0), glm::vec3(-90,-90,0), glm::vec3(90,90,0)));
    params.add(cameraOrientation.set("Camera orientation", glm::vec3(0,0,0), glm::vec3(-180,-90,0), glm::vec3(180,90,0) ));
    
    load();
    
    ofAddListener(params.parameterChangedE(), this, &Visualiser3D::paramsChanged);
   
}
Visualiser3D :: ~Visualiser3D() {
    ofRemoveListener(params.parameterChangedE(), this, &Visualiser3D::paramsChanged);
}



void Visualiser3D :: update() {
    //update view stuff
    
    bool needsSave = dirty;
    for(Laser3DVisualObject* laser3D : laser3Ds) {
        if(laser3D->dirty) {
            laser3D->dirty = false;
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
    while(lasers.size()>laser3Ds.size()) {
        laser3Ds.push_back(new Laser3DVisualObject());
        // load?
    }
    
    if((!visFbo.isAllocated()) || (visFbo.getWidth()!=rect.getWidth()) || (visFbo.getHeight()!=rect.getHeight())) {
        visFbo.allocate(rect.getWidth(), rect.getHeight(), GL_RGBA, 4);
    }
    
    visFbo.begin();
    
    ofBackground(0);
    
    // ofSetDepthTest(true);
    ofSetupScreenPerspective(visFbo.getWidth(), visFbo.getHeight(), cameraFov);
    
    ofPushMatrix();
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    

    ofTranslate(visFbo.getWidth()/2,visFbo.getHeight()/2,0);
    
    
    ofRotateYDeg(cameraOrientation.get().x);
    ofRotateXDeg(cameraOrientation.get().y);
   
    ofTranslate(0,0,-cameraDistance); // pull back 1000 units
    
    ofRotateYDeg(cameraOrbit.get().x);
    ofRotateXDeg(cameraOrbit.get().y);
   
    
    ofPoint mousepos(ofGetMouseX(), ofGetMouseY());
    
    ofRectangle fboRect(0,0,visFbo.getWidth(), visFbo.getHeight());
    if(fboRect.inside(mousepos)) {
        ofTranslate(ofMap(mousepos.x, 0, fboRect.getWidth(), 30,-30), ofMap (mousepos.y, 0, fboRect.getWidth(), 100,-100) );
    }
    
    
    drawGrid();
    
    for(size_t i= 0; i<lasers.size(); i++) {
        
        ofPushStyle();
        ofPushMatrix();
        
        ofxLaser::Laser& laser = *lasers.at(i);
        Laser3DVisualObject& laser3D = *laser3Ds.at(i);
        vector<ofxLaser::Point>& laserPoints = laser.getLaserPoints();
        
        //move to the laser position
        ofTranslate(laser3D.position);
        ofRotateXDeg(laser3D.orientation.get().x);
        ofRotateYDeg(laser3D.orientation.get().y);
        ofRotateZDeg(laser3D.orientation.get().z);
        ofNoFill();
        ofSetLineWidth(1);
        ofSetColor(0,100,0);
        
        ofDrawBox(0, 0, -50, 70, 50, 160);
        
        ofMesh laserMesh;
        
        for(int i = 1; i<laserPoints.size(); i++) {
            
            
            ofxLaser::Point& lp1 = laserPoints[i-1];
            ofxLaser::Point& lp2 = laserPoints[i];
            ofColor colour1 = lp1.getColour();
            ofColor colour2 = lp2.getColour();
            
            // find 3 points that make a triangle
            // the point of the triangle is at the laser position
            // then the two corners are points in the laser path
            // Z is negative? into the screen
            // laser positions are relative to 0,0,0 which should be in the
            // distance in the centre of the screen.
            ofPoint p1, p2;
            //p1.x = ofMap(lp1.x, 0, 800, -2000, 2000);
            //p1.y = ofMap(lp1.y, 0, 800, -2000, 2000);
            //p2.x = ofMap(lp2.x, 0, 800, -2000, 2000);
            //p2.y = ofMap(lp2.y, 0, 800, -2000, 2000);
            p1.z = 2500;
            p2.z = 2500;
            p1.rotate(ofMap(lp1.x, 0, 800, -laser3D.horizontalRangeDegrees/2, laser3D.horizontalRangeDegrees/2), ofPoint(0,1,0));
            p2.rotate(ofMap(lp2.x, 0, 800, -laser3D.horizontalRangeDegrees/2, laser3D.horizontalRangeDegrees/2), ofPoint(0,1,0));
            p1.rotate(ofMap(lp1.y, 0, 800, -laser3D.verticalRangeDegrees/2, laser3D.horizontalRangeDegrees/2), ofPoint(-1,0,0));
            p2.rotate(ofMap(lp2.y, 0, 800, -laser3D.verticalRangeDegrees/2, laser3D.horizontalRangeDegrees/2), ofPoint(-1,0,0));
            
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
        for(int x = -5000; x<5000; x+=100) {
            grid.addVertex(glm::vec3(x, 400, -2000));
            grid.addVertex(glm::vec3(x, 400, 2000));
            grid.addColor(ofColor(0, 0, 0));
            grid.addColor(ofColor(0, 50, 0));
        }
        for(int z = -2000; z<2000; z+=100) {
            grid.addVertex(glm::vec3(-5000, 400, z));
            grid.addVertex(glm::vec3(5000, 400, z));
            grid.addColor(ofColor(0, ofMap(z,-2000,2000,0,50), 0));
            grid.addColor(ofColor(0, ofMap(z,-2000,2000,0,50), 0));
        }

    }
    grid.draw();
    
}


void Visualiser3D ::load() {
    ofJson json = ofLoadJson("ofxLaser/Visualiser3D.json");
    ofDeserialize(json, params);
    ofJson& laser3DsJson = json["laser3Ds"];
    while(laser3Ds.size()<laser3DsJson.size()) laser3Ds.push_back(new Laser3DVisualObject()); // laser3DsJson.size());
    for(int i = 0; i<laser3Ds.size(); i++) {
        ofDeserialize(laser3DsJson[i], laser3Ds[i]->visual3DParams);
    }
    
}
void Visualiser3D ::save() {
    ofJson json;
    ofSerialize(json, params);
    ofJson& laser3DsJson = json["laser3Ds"];
    
    for(int i = 0; i<laser3Ds.size(); i++) {
        ofJson laser3dJson;
        ofSerialize(laser3dJson, laser3Ds[i]->visual3DParams);
        
        laser3DsJson.push_back(laser3dJson);
    }

    ofSaveJson("ofxLaser/Visualiser3D.json",json);
    
    ofLogNotice("Visualiser3D json : " ) << json.dump(3);
    
}


void Visualiser3D ::drawUI(){
    
    UI::startWindow("3D Visualiser", ImVec2(100,100));
    
    UI::addFloatSlider(cameraDistance); //.set("Camera FOV", 45,10,120))
    UI::addFloatSlider(cameraFov); //.set("Camera FOV", 45,10,120));
    
    UI::addFloat3Slider(cameraOrbit);//.set("Camera position", glm::vec3(0,-2000,0)));
    UI::addFloat3Slider(cameraOrientation);//.set("Camera orientation", glm::vec3(0,0,0)));
    
    ImGui::Separator();
    
    
    for(int i = 0; i<numLasers && i<laser3Ds.size(); i++) {
        ImGui::Separator();
        ImGui::Text("Laser %d", i+1);
        Laser3DVisualObject* laser3D = laser3Ds[i];
        UI::addFloat3Slider(laser3D->position, "%.0f", 1, "Position##"+ofToString(i));
        UI::addFloat3Slider(laser3D->orientation, "%.0f", 1, "Orientation##"+ofToString(i));
    }
    
    
    
    
    UI::endWindow();
}
