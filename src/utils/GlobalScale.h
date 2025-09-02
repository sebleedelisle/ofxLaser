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
        return tmpxscale * multiplier; //  glfwWindow->getPixelScreenCoordScale();
        
        
    }
    static bool isHiDPI() {
        ofAppGLFWWindow* glfwWindow = ((ofAppGLFWWindow*)(ofGetWindowPtr()));
        float tmpxscale, tmpyscale;
        glfwGetWindowContentScale(glfwWindow->getGLFWWindow(), &tmpxscale, &tmpyscale);
        return tmpxscale>1.8; 
        
    }
    
    static int getScalePercentInt() {
        return round(getScale()*100);
    }
    
    static float getMouseWheelSpeed() {
#ifdef TARGET_OSX
        return mouseWheelSpeedInternal;
#else
        return mouseWheelSpeedInternal*2;
#endif
    }
    static void setMouseWheelSpeed(float newspeed) {
#ifdef TARGET_OSX
        mouseWheelSpeedInternal = newspeed;
#else
        mouseWheelSpeedInternal = newspeed/2;
#endif
    }
    
    static float multiplier;
    static float mouseWheelSpeedInternal;
    
};
