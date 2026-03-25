//
//  ofxLaserUIState.h
//
//  UI-only state extracted from Manager. Keeps the model layer free
//  of presentation concerns.
//
//  Created during architecture alignment with Liberation patterns.
//

#pragma once

#include "ofMain.h"

enum ofxLaserViewMode {
    OFXLASER_VIEW_3D,     // show 3D visualiser view (Extra only)
    OFXLASER_VIEW_CANVAS, // show default canvas view
    OFXLASER_VIEW_OUTPUT, // show laser output system
};

namespace ofxLaser {

struct UIState {

    // --- Visibility / window state ---
    bool guiIsVisible            = true;
    bool guiIsMouseDisabled      = false;
    bool windowActive            = true;

    // --- View mode ---
    ofxLaserViewMode viewMode    = OFXLASER_VIEW_CANVAS;

    // --- Selection ---
    int selectedLaserIndex       = 0;

    // --- Panel windows ---
    bool showCopySettingsWindow      = false;
    bool showDacAssignmentWindow     = false;
    bool showAVBWindow               = false;
    bool showEditScannerPreset       = false;

    // --- Layout ---
    int guiLaserSettingsPanelWidth   = 320;
    int guiSpacing                   = 8;
    int menuBarHeight                = 20;
    int iconBarHeight                = 32;
    bool overlayIconsVisible         = true;
    bool renderMainMenu              = true;
    bool renderIconBar               = true;
    bool autoSizeToScreen            = true;
    ofRectangle windowRectangle;

};

} // namespace ofxLaser
