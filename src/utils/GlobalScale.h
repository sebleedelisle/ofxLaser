//
//  GlobalScale.h
//  Liberation
//
//  Created by Seb Lee-Delisle on 18/04/2024.
//
#pragma once
#include "ofMain.h"
#include "GLFW/glfw3.h"

class GlobalScale {
    public :
//    GlobalScale();
//    
//    static GlobalScale * instance();
//    static GlobalScale * globalScale;
//    
    
    static float getScale() {
        //ofLogNotice() << ((ofAppGLFWWindow*)(ofGetWindowPtr()))->getPixelScreenCoordScale();
       // ofLogNotice() << ((ofAppGLFWWindow*)(ofGetWindowPtr()))->glfwGetWindowContentScale();
        
        ofAppGLFWWindow* glfwWindow = ((ofAppGLFWWindow*)(ofGetWindowPtr()));
        
        
        float tmpxscale, tmpyscale;
        //glfWindow->getGLFWWindow()
        glfwGetWindowContentScale(glfwWindow->getGLFWWindow(), &tmpxscale, &tmpyscale);
        std::cout << tmpxscale << " " << tmpyscale << endl;
//        
        return glfwWindow->getPixelScreenCoordScale();
        
        
    }
    
};
