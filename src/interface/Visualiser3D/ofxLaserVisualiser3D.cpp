//
//  ofxLaserVisualiser3D.cpp
//
//  Created by Seb Lee-Delisle on 10/06/2022.
//


#include "ofxLaserVisualiser3D.h"

using namespace ofxLaser;


Visualiser3D :: Visualiser3D() {
    
    params.add(brightness.set("Brightness adjustment", 1,0.1,10));
    params.add(cameraDistance.set("Camera distance", 50,0,1000));
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
    ofSetupScreenPerspective(800, 350, cameraFov,0.1,100000);
    
    float eyeY = 800 / 2;
    float halfFov = PI * cameraFov / 360;
    float theTan = tanf(halfFov);
    float dist = eyeY / theTan;
    //ofLogNotice()<<dist;
    
    ofPushMatrix();
    ofPushStyle();
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    
    //float scalefactor = (float)rect.getWidth()/800.0f;
   // ofScale(scalefactor);
    ofTranslate(400,175,600);
    
    
    ofRotateYDeg(cameraOrientation.get().x);
    ofRotateXDeg(cameraOrientation.get().y);
   
    ofTranslate(0,0,-cameraDistance); // pull back z units
    
    ofRotateXDeg(cameraOrbit.get().y);
    ofRotateYDeg(cameraOrbit.get().x);
   
    
    ofPoint mousepos(ofGetMouseX(), ofGetMouseY());
    
//    ofRectangle fboRect(0,0,visFbo.getWidth(), visFbo.getHeight());
//    if(fboRect.inside(mousepos)) {
//        ofTranslate(ofMap(mousepos.x, 0, fboRect.getWidth(), 30,-30), ofMap (mousepos.y, 0, fboRect.getWidth(), 100,-100) );
//    }
    
    
    drawGrid();
    
    for(size_t i= 0; i<lasers.size(); i++) {
        
        ofPushStyle();
        ofPushMatrix();
        
        ofxLaser::Laser& laser = *lasers.at(i);
        Laser3DVisualObject& laser3D = *laser3Ds.at(i);
        //vector<ofxLaser::Point>& laserPoints = laser.getLaserPoints();
        vector<glm::vec3>& points = laser.previewPathMesh.getVertices();
        vector<ofColor>& colours = laser.previewPathColours;

        float brightnessfactor = MAX(0.01f,5.0f/(float)points.size()) * brightness;
        
        //move to the laser position
        ofTranslate(laser3D.position);
        ofRotateXDeg(laser3D.orientation.get().x);
        ofRotateYDeg(laser3D.orientation.get().y);
        ofRotateZDeg(laser3D.orientation.get().z);
        
        ofNoFill();
        ofSetLineWidth(1);
        ofSetColor(0,100,0);
        
        ofDrawBox(0, 0, -5, 7, 5, 10);
        
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
    
    //ofLogNotice("Visualiser3D json : " ) << json.dump(3);
    
}


void Visualiser3D ::drawUI(){
    
    UI::startWindow("3D Visualiser", ImVec2(100,100), ImVec2(500,0));
    
    UI::addFloatSlider(brightness); //.set("Camera FOV",
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
