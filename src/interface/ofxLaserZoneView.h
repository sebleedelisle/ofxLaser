//
//  LaserZoneView.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 13/01/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserLaser.h"
#include "ofxLaserViewPort.h"
#include "ofxLaserZoneUIQuad.h"

namespace ofxLaser {
class LaserZoneView : public ViewPort {
    
    public :
    // initialise :
    LaserZoneView();
    LaserZoneView(Laser * laser);
    ~LaserZoneView(); 
        
    void draw();
    void drawLaserPath();
    bool updateZones(); 

    void setGrid(bool snaptogrid, int gridsize);
    
  
  
    
    virtual void mouseMoved(ofMouseEventArgs &e) override;
    virtual bool mousePressed(ofMouseEventArgs &e) override;
    virtual void mouseDragged(ofMouseEventArgs &e) override;
    virtual void mouseReleased(ofMouseEventArgs &e) override; 
    

    
    // set source and output rect? Or is that in draw?
    // draw
    // for now let's not zoom, let's just concentrate on drawing stuff!
    //
    //

    // one view per laser
    // zone object
    // periodically needs to update from the laser object
    
    
    // settings :
    // show path preview
    // show dots
    // visible / enabled
    
    Laser* laser;
    
    ofMesh gridMesh; 
    int gridSize = 1;
    bool snapToGrid = false;
    
    vector<ZoneUIQuad> zoneUiQuads; 
    
    

    
    
};
}
