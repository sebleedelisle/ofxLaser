//
//  ofxLaserCanvasViewController.cpp
//
//  Created by Seb Lee-Delisle on 11/02/2023.
//
//

#include "ofxLaserCanvasViewController.h"

using namespace ofxLaser;

bool CanvasViewController :: updateZonesFromUI(vector<InputZone*>& zones){
    //return false;
    for(int i = 0; i<uiElements.size(); i++) {
        if(zones.size()>i) {
            InputZone& zone = *zones[i];
            MoveablePoly& poly = *uiElements[i];
            vector<glm::vec2*> points = poly.getPoints();
            
            // TODO better way to get MoveablePoly as rect
            float x = points[0]->x;
            float y = points[0]->y;
            float w = points[2]->x - points[0]->x;
            float h = points[2]->y - points[0]->y;

            zone.set( x, y, w, h); 
            
        }
        
    }
    
    return true;
}
void CanvasViewController :: updateUIFromZones(const vector<InputZone*>& zones) {
    for(int i = 0; i<zones.size(); i++) {
        MoveablePoly* poly;
        if(uiElements.size()<=i) {
            poly = new MoveablePoly();
            uiElements.push_back(poly);
            uiElementsSorted.push_back(poly);
            poly->setHue(220); 
            
        } else {
            poly = uiElements[i];
        
        }
        vector<glm::vec2> points;
        for(int j = 0; j<4; j++) {
            int index = j;
            if(index>1) index = 3 - (index%2);
            points.push_back(zones[i]->handles[index]);

        }
        //DragHandle& handles =  zones[i]->handles;
        poly->setFromPoints(points); //(vector<glm::vec2>));
        poly->setLabel(ofToString(i+1));
            
    }
    //vector<MoveablePoly*>::iterator it = uiElements.begin();
    
    for(int i = zones.size(); i<uiElements.size(); i++) {
       
        uiElementsSorted.erase(std::remove(uiElementsSorted.begin(), uiElementsSorted.end(), uiElements[i]), uiElementsSorted.end());
        delete uiElements[i];
    }
    uiElements.resize(zones.size());
    
    
    
}
