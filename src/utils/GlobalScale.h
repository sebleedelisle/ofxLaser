//
//  GlobalScale.h
//  Liberation
//
//  Created by Seb Lee-Delisle on 18/04/2024.
//
#pragma once
#include "ofMain.h"


class GlobalScale {
    public :
//    GlobalScale();
//    
//    static GlobalScale * instance();
//    static GlobalScale * globalScale;
//    
    
    static float getScale() {
        ofLogNotice() << ((ofAppGLFWWindow*)(ofGetWindowPtr()))->getPixelScreenCoordScale();
        //ofLogNotice() << ((ofAppGLFWWindow*)(ofGetWindowPtr()))->glfwGetWindowContentScale();
        return ((ofAppGLFWWindow *)(ofGetWindowPtr()))->getPixelScreenCoordScale();
        
        
    }
    
};
