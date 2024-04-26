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

    
    static float getScale() {

        //return 1.2;//.2;
        ofAppGLFWWindow* glfwWindow = ((ofAppGLFWWindow*)(ofGetWindowPtr()));
        float tmpxscale, tmpyscale;
        glfwGetWindowContentScale(glfwWindow->getGLFWWindow(), &tmpxscale, &tmpyscale);
        //std::cout << tmpxscale << " " << tmpyscale << endl;
//        
        return tmpxscale; //  glfwWindow->getPixelScreenCoordScale();
        
        
    }
    
};
