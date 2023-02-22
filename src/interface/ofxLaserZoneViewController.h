//
//  LaserZoneView.h
//
//
//  Created by Seb Lee-Delisle on 13/01/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserLaser.h"
#include "ofxLaserViewWithMoveables.h"
#include "ofxLaserZoneUiQuad.h"
#include "ofxLaserZoneUiLine.h"
#include "ofxLaserMaskUiQuad.h"

namespace ofxLaser {
class LaserZoneViewController : public ViewWithMoveables {
    
    public :
    // initialise :
    LaserZoneViewController();
    LaserZoneViewController(Laser * laser);
    ~LaserZoneViewController();
    
    ZoneUiBase* getZoneInterfaceForOutputZone(OutputZone* outputZone);
    OutputZone* getOutputZoneForZoneUI(ZoneUiBase* zoneUi, vector<OutputZone*>& outputZones);
    
    bool createZoneUiForOutputZone(OutputZone* outputZone);
    void deselectAllButThis(MoveablePoly* uielement) override;
    
    bool doesAltZoneExistForZoneIndex(int zoneIndex); 
    
    bool update() override;
    void draw() override;
    
    
    void setGrid(bool snaptogrid, int gridsize) override;
    
    
    void drawImGui(); 
    void drawLaserPath();
    bool updateZones();
    bool updateMasks();
    int getLaserIndex(); 

    void resetUiElements();
    void moveMasksToBack();

    Laser* laser;
    
    vector<ZoneUiBase*> zoneUis;
    vector<ZoneUiBase*> zoneUisSorted;
    
    vector<MaskUiQuad*> maskUis;
    vector<MaskUiQuad*> maskUisSorted;
 
};
}
