//
//  ofxLaserZoneUiLine.h
//
//
//  Created by Seb Lee-Delisle on 06/02/2023.
//
//

#pragma once

#include "ofxLaserZoneUiBase.h"
#include "ofxLaserZoneTransformLineData.h"
#include "SebUtils.h"

namespace ofxLaser {
class ZoneUiLine : public ZoneUiBase {
    
    public :
    ZoneUiLine();
    
    bool updateDataFromUi(std::shared_ptr<OutputZone>& outputZone) override;
    bool updateFromData(std::shared_ptr<OutputZone>& outputZone) override;
    glm::vec2 getClosestPointOnLine(glm::vec2 pos); 
    int getClosestPointIndexToPosition(glm::vec2 pos);
    
    bool deleteVertex(int vertexindex);
    void addVertex(glm::vec2 point);
    
    bool mousePressed(ofMouseEventArgs &e) override;
    
    void draw() override;
    
    int getNumPoints();
    glm::vec2 getPointAtIndex(int index); 
    void updatePoly() override;
    bool autoSmooth = true; 
    ofPolyline linePoly;
    
    int vertexToDelete =-1;
    vector<glm::vec2> pointsToAdd; 
    
   //vector<DragHandle*> anchorPoints; // bezier anchor points
   // vector<DragHandle*> controlPoints; // bezier control points
   
};
}
