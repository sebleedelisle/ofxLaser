//
//  ofxLaserVisualiser3D.h
//  example_LaserEffects
//
//  Created by Seb Lee-Delisle on 10/06/2022.
//

#pragma once
#include "ofxLaserManager.h"

namespace ofxLaser {

class Visualiser3D {
    
    public :
    
    void draw(const ofRectangle& rect, const vector<Laser*>& lasers) {
        
            
        if((!visFbo.isAllocated()) || (visFbo.getWidth()!=rect.getWidth()) || (visFbo.getHeight()!=rect.getHeight())) {
            visFbo.allocate(rect.getWidth(), rect.getHeight(), GL_RGBA, 4);
        }
        
        visFbo.begin();

        ofBackground(0);
        
       // ofSetDepthTest(true);
        ofSetupScreenPerspective(visFbo.getWidth(), visFbo.getHeight(), 45);
        
        ofPushMatrix();
        ofPushStyle();
        ofEnableBlendMode(OF_BLENDMODE_ADD);
        
       
        
        ofTranslate(visFbo.getWidth()/2,visFbo.getHeight()/2, -2000); // pull back 1000 units
        ofPoint mousepos(ofGetMouseX(), ofGetMouseY());
        
        
        
        
        ofRectangle fboRect(0,0,visFbo.getWidth(), visFbo.getHeight());
        if(fboRect.inside(mousepos)) {
            ofTranslate(ofMap(mousepos.x, 0, fboRect.getWidth(), 30,-30), ofMap (mousepos.y, 0, fboRect.getWidth(), 100,-100) );
        }
        
        
        
        ofMesh grid;
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
        grid.draw();
        
        
        
        
        
        for(size_t i= 0; i<lasers.size(); i++) {
                
            ofPushStyle();
            ofPushMatrix();
            
            
            
            ofxLaser::Laser& laser = *lasers.at(i);
            
            vector<ofxLaser::Point>& laserPoints = laser.getLaserPoints();
            
            //move to the laser position
            ofTranslate(laser.position);
            ofRotateXDeg(laser.orientation.get().x);
            ofRotateYDeg(laser.orientation.get().y);
            ofRotateZDeg(laser.orientation.get().z);
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
                p1.rotate(ofMap(lp1.x, 0, 800, -laser.horizontalRangeDegrees/2, laser.horizontalRangeDegrees/2), ofPoint(0,1,0));
                p2.rotate(ofMap(lp2.x, 0, 800, -laser.horizontalRangeDegrees/2, laser.horizontalRangeDegrees/2), ofPoint(0,1,0));
                p1.rotate(ofMap(lp1.y, 0, 800, -laser.verticalRangeDegrees/2, laser.horizontalRangeDegrees/2), ofPoint(-1,0,0));
                p2.rotate(ofMap(lp2.y, 0, 800, -laser.verticalRangeDegrees/2, laser.horizontalRangeDegrees/2), ofPoint(-1,0,0));

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
    
    
    ofFbo visFbo;
    
    
    
    
    
};
}

