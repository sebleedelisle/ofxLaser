//
//  LaserZoneView.h
//
//
//  Created by Seb Lee-Delisle on 13/01/2023.
//



// REFACTOR 4 Oct 23
// We need an abstract class that can handle multiple groups of
// different extensions of MoveablePoly
//
// Each MoveablePoly subclass needs a way to be updated with a data class.
//
// We need to store each subclass type in an array, argh c++ amirite?
// We need a map to store lists of subclasses, indexed by their type
//
// We need options for sorting all of the MoveablyPolys, either all as one
// or within groups of their type
//
// We need a standard way of associating data objects with their moveable
// counterparts - could those data objects inherit a subclass ? They're
// gonna need to have some kind of rectangle / point data to update right ?
//
// I think we can get rid of the clumsy sorted / non-sorted vectors. The only
// reason we had the sorted vector was as a way to associate data objects
// with MoveablePoly objects.
//
// If we establish a different way of doing this, some kind of label, then
// we don't need the two systems. 


#pragma once

#include "ofMain.h"
#include "ofxLaserManagerBase.h"
#include "ofxLaserViewWithMoveables.h"
#include "ofxLaserZoneUiQuad.h"
#include "ofxLaserZoneUiQuadComplex.h"
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
    
    bool doesAltZoneExistForZoneIndex(ZoneId zoneId); 
    
    bool update() override;
    void draw() override;
    
    
    void setGrid(bool snaptogrid, int gridsize, bool visible) override;
    
    
    void drawImGui(); 
    void drawLaserPath();
    bool updateZones();
    bool updateMasks();
    int getLaserIndex(); 

    void resetUiElements();
    void moveMasksToBack();

    Laser* laser;
    
    // i think multiple types are built in now, right? TODO refactor
    vector<ZoneUiBase*> zoneUis;
    vector<ZoneUiBase*> zoneUisSorted;
    
    vector<MaskUiQuad*> maskUis;
    vector<MaskUiQuad*> maskUisSorted;
    
    static int objectCount; 
 
};
}
