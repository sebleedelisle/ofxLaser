//
//  ofxLaserZoneViewController.h
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
#include "ofxLaserBaseView.h"
#include "ofxLaserBaseController.h"
#include "ofxLaserZoneUIQuad.h"
#include "ofxLaserZoneUIQuadComplex.h"
#include "ofxLaserMaskUIQuad.h"
namespace ofxLaser {
class LaserZoneViewController : public ViewWithMoveables, public LaserBaseView, public LaserBaseController {
    
    public :
    // initialise :
    LaserZoneViewController();
    LaserZoneViewController(std::shared_ptr<Laser>& laser);
    ~LaserZoneViewController();
    
    virtual void drawMoveables() override;
    virtual bool mousePressed(ofMouseEventArgs &e) override;
    
    std::shared_ptr<ZoneUIBase> getZoneInterfaceForOutputZone(std::shared_ptr<OutputZone>& outputZone);
    std::shared_ptr<OutputZone> getOutputZoneForZoneUI(std::shared_ptr<ZoneUIBase>& zoneUi);
    std::shared_ptr<ZoneUIBase> getSelectedZoneUi();
    std::shared_ptr<OutputZone> getSelectedOutputZone();
    
    bool createZoneUIForOutputZone(std::shared_ptr<OutputZone>& outputZone);
    void deselectAllButThis(std::shared_ptr<MoveablePoly>& uielement) override;
    void deselectAll() override;
    bool update() override;
    void draw() override;
    
    
    void setGrid(bool snaptogrid, int gridsize, bool visible) override;
    
    void drawLaserPath();
    bool updateZones();
    bool updateMasks();
    int getLaserIndex(); 
    
    std::shared_ptr<Laser>& getLaser() {
        return laser;
    }
    void resetUiElements();
    void moveMasksToBack();

    // ============================================================
    // LaserBaseView signal overrides
    // ============================================================
    void onZonesChanged() override;
    void onMasksChanged() override;

    protected :
    
    std::shared_ptr<Laser> laser;
    
    vector<std::shared_ptr<ZoneUIBase>> zoneUis;
    vector<std::shared_ptr<ZoneUIBase>> zoneUisSorted;
    
    vector<std::shared_ptr<MaskUIQuad>> maskUis;
    vector<std::shared_ptr<MaskUIQuad>> maskUisSorted;
    
    static int objectCount; 
 
};
}
