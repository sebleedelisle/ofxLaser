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
#include "ofxLaserScrollableView.h"
#include "ofxLaserZoneUIQuad.h"
#include "ofxLaserZoneUIBase.h"

namespace ofxLaser {
class LaserZoneViewController : public ScrollableView {
    
    public :
    // initialise :
    LaserZoneViewController();
    LaserZoneViewController(Laser * laser);
    ~LaserZoneViewController();
    
    ZoneUiBase* getZoneInterfaceForOutputZone(OutputZone* outputZone);
    OutputZone*  getOutputZoneForZoneUI(ZoneUiBase* zoneUi, vector<OutputZone*>& outputZones);
    bool createZoneUiForOutputZone(OutputZone* outputZone);
    void updateSelected(ZoneUiBase* zoneUi); 
    
    bool update() override;
    
    void draw();
    void drawImGui(); 
    void drawLaserPath();
    bool updateZones();
    bool updateMasks();

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
    
    vector<ZoneUiBase*> zoneUis;
    vector<ZoneUiBase*> zoneUisSorted;
    
    vector<MoveablePoly*> maskUis;

};
}
