//
//  ofxLaserShapeTargetCanvas.cpp
//
//  Created by Seb Lee-Delisle on 23/02/2023.
//
//

#include "ofxLaserShapeTargetCanvas.h"

using namespace ofxLaser;

ShapeTargetCanvas :: ShapeTargetCanvas() {
    zoneGroup = 0;
    type = ZoneId::CANVAS;
} 

ZoneId ShapeTargetCanvas :: addInputZone(float x, float y, float w, float h){
    std::shared_ptr<InputZone> newcanvaszone = std::make_shared<InputZone>( x, y, w, h);
    addZoneIdObject(newcanvaszone);
    return newcanvaszone->getZoneId();
}

bool ShapeTargetCanvas :: addGuideImage(string filename) {
    
    //guideImages.resize(guideImages.size()+1);
    guideImages.push_back(std::make_shared<GuideImage>());
    std::shared_ptr<GuideImage>& guide = guideImages.back();
    
    if(guide->load(filename)) {
        
        guide->setRectFromImage();
        guide->stretchToFit(getBounds());
        return true;
    } else {
        guideImages.pop_back();
        return false;
    }
    
    
}



bool ShapeTargetCanvas :: addZoneByJson(ofJson& json) {
    //cout << json.dump(3) << endl;
    std::shared_ptr<InputZone> zoneIdObject = std::make_shared<InputZone>();
    if(zoneIdObject->deserialize(json)) {
        addZoneIdObject(zoneIdObject);
        return true;
    } else {
        return false;
    }
}

vector<std::shared_ptr<Shape>> ShapeTargetCanvas :: getShapesForZoneId(ZoneId& zoneid){
    
    std::shared_ptr<InputZone> inputZone = getInputZoneForZoneId(zoneid);
    
    vector<std::shared_ptr<Shape>> newshapes;
    
    if(inputZone) {

        for(std::shared_ptr<Shape>& shape : shapes) { //size_t i= 0; i<shapes.size(); i++) {
            
            // if (zone should have shape) then
            if((!shape->isEmpty()) && shape->intersectsRect(inputZone->getRect())) {
                newshapes.push_back(shape);
            }
        }
    } else {
        ofLogError("ShapeTargetCanvas :: getShapesForZoneId - missing zone id");
    }
    return newshapes;
    
}

std::shared_ptr<InputZone> ShapeTargetCanvas :: getInputZoneForZoneId(ZoneId& zoneid) {
    std::shared_ptr<ObjectWithZoneId> zoneobject = getObjectForZoneId(zoneid);
    return std::dynamic_pointer_cast<InputZone>(zoneobject);
     
}

std::shared_ptr<InputZone> ShapeTargetCanvas :: getInputZoneForZoneIdUid(string& uid) {
    std::shared_ptr<ObjectWithZoneId> zoneobject = getObjectForZoneIdUid(uid);
    return std::dynamic_pointer_cast<InputZone>(zoneobject);
     
}

std::shared_ptr<InputZone> ShapeTargetCanvas :: getInputZoneForZoneIndex(int index) {
    std::shared_ptr<ObjectWithZoneId> zoneobject = getObjectAtIndex(index);
    return std::dynamic_pointer_cast<InputZone>(zoneobject);
     
}

std::shared_ptr<GuideImage> ShapeTargetCanvas :: getGuideImageForUid(string& uid) {
    for(int i = 0; i<guideImages.size(); i++) {
        string id = "guide" + ofToString(i);
        if(id == uid) return guideImages.at(i);
    }
    return nullptr;
}

bool ShapeTargetCanvas :: deleteGuideImage(std::shared_ptr<GuideImage>& guideImage){
    return SebUtils::removeElementFromVector(guideImages, guideImage);
//    for(int i = 0; i<guideImages.size(); i++) {
//        if(&guideImages.at(i) == guideImage) {
//            auto it = guideImages.begin()+i;
//            guideImages.erase(it);
//            return true;
//        }
//        
//        
//    }
//    return false;
}




vector<std::shared_ptr<InputZone>> ShapeTargetCanvas :: getInputZones() {
    // these seems... substandard haha
    vector<std::shared_ptr<InputZone>> inputZones;
    for(std::shared_ptr<ObjectWithZoneId> zoneobject : zoneIdObjects) {
        inputZones.push_back(std::dynamic_pointer_cast<InputZone>(zoneobject));
    }
    return inputZones;
}

vector<std::shared_ptr<GuideImage>>& ShapeTargetCanvas :: getGuideImages() {
    return guideImages;
}

void ShapeTargetCanvas :: serialize(ofJson& json)  {
    ZoneIdContainer ::serialize(json);
    ofJson& containerJson = json["zoneidcontainer"];
    containerJson["canvas_width"] = getBounds().getWidth();
    containerJson["canvas_height"] = getBounds().getHeight();
    containerJson["zone_brightness"] = zoneBrightness;
    ofJson& guideImagesJson = containerJson["guide_images"];
    
    for(std::shared_ptr<GuideImage>& guideImage : guideImages) {
        ofJson jsonimage;
        guideImage->serialize(jsonimage);
        guideImagesJson.push_back(jsonimage);
    }
}

bool ShapeTargetCanvas :: deserialize(ofJson& json) {
    //ofLogNotice() << json.dump(3); 
    if(ZoneIdContainer :: deserialize(json) ) {
       // it has to contain this otherwise the parent deserialize would return false
        ofJson& containerJson = json["zoneidcontainer"];
        if(containerJson.contains("canvas_width") && containerJson.contains("canvas_height")) {
         
            setBounds(0,0,containerJson["canvas_width"].get<int>(), containerJson["canvas_height"].get<int>());
            
            if(containerJson.contains("zone_brightness")) zoneBrightness = containerJson["zone_brightness"].get<int>();
            
            if(containerJson.contains("guide_images")) {
                ofJson& imagejson = containerJson["guide_images"];
                guideImages.clear();
                int numguides = imagejson.size();
                guideImages.resize(numguides);
                for(int i = 0; i<numguides; i++) {
                    guideImages[i] = std::make_shared<GuideImage>();
                    guideImages[i]->deserialize(imagejson.at(i));
                }
            }
            
            return true;
        } else {
            return false;
        }
        
    } else {
        return false;
    }
    
  
    
}
