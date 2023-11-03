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
    InputZone* newcanvaszone = new InputZone( x, y, w, h);
    addZoneIdObject(newcanvaszone);
    return newcanvaszone->getZoneId();
}

bool  ShapeTargetCanvas :: addGuideImage(string filename) {
    
    guideImages.resize(guideImages.size()+1);
    GuideImage& guide = guideImages.back();
    
    if(guide.load(filename)) {
        guide.setRectFromImage();
        return true;
    } else {
        guideImages.pop_back();
        return false;
    }
    
    
}



bool ShapeTargetCanvas :: addZoneByJson(ofJson& json) {
    cout << json.dump(3) << endl;
    InputZone* zoneIdObject = new InputZone();
    if(zoneIdObject->deserialize(json)) {
        addZoneIdObject(zoneIdObject);
        return true;
    } else {
        delete zoneIdObject;
        return false;
    }
}

vector<Shape*> ShapeTargetCanvas :: getShapesForZoneId(ZoneId& zoneid){
    
    InputZone* inputZone = getInputZoneForZoneId(zoneid);
    
    vector<Shape*> newshapes;
    
    if(inputZone!=nullptr) {

        for(Shape* shape : shapes) { //size_t i= 0; i<shapes.size(); i++) {
            
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

InputZone* ShapeTargetCanvas :: getInputZoneForZoneId(ZoneId& zoneid) {
    ObjectWithZoneId* zoneobject = getObjectForZoneId(zoneid);
    return dynamic_cast<InputZone*>(zoneobject);
     
}

InputZone* ShapeTargetCanvas :: getInputZoneForZoneIdUid(string& uid) {
    ObjectWithZoneId* zoneobject = getObjectForZoneIdUid(uid);
    return dynamic_cast<InputZone*>(zoneobject);
     
}

InputZone* ShapeTargetCanvas :: getInputZoneForZoneIndex(int index) {
    ObjectWithZoneId* zoneobject = getObjectAtIndex(index);
    return dynamic_cast<InputZone*>(zoneobject);
     
}

GuideImage* ShapeTargetCanvas :: getGuideImageForUid(string& uid) {
    for(int i = 0; i<guideImages.size(); i++) {
        string id = "guide" + ofToString(i);
        if(id == uid) return &guideImages.at(i);
    }
    return nullptr;
}

bool ShapeTargetCanvas :: deleteGuideImage(GuideImage* guideImage){
    
    for(int i = 0; i<guideImages.size(); i++) {
        if(&guideImages.at(i) == guideImage) {
            auto it = guideImages.begin()+i;
            guideImages.erase(it);
            return true;
        }
        
        
    }
    return false;
        
      
    
    
}




vector<InputZone*> ShapeTargetCanvas :: getInputZones() {
    // these seems... substandard haha
    vector<InputZone*> inputZones;
    for(ObjectWithZoneId* zoneobject : zoneIdObjects) {
        inputZones.push_back(dynamic_cast<InputZone*>(zoneobject));
    }
    return inputZones;
}

vector<GuideImage*> ShapeTargetCanvas :: getGuideImages() {
    vector<GuideImage*> guideImagesToSend;
    for(GuideImage& guide : guideImages)  {
        guideImagesToSend.push_back(&guide);
    }
    return guideImagesToSend;
}

void ShapeTargetCanvas :: serialize(ofJson& json)  {
    ZoneIdContainer ::serialize(json);
    ofJson& containerJson = json["zoneidcontainer"];
    containerJson["canvas_width"] = getBounds().getWidth();
    containerJson["canvas_height"] = getBounds().getHeight();
    containerJson["zone_brightness"] = zoneBrightness;
    ofJson& guideImagesJson = containerJson["guide_images"];
    
    for(GuideImage& guideImage : guideImages) {
        ofJson jsonimage;
        guideImage.serialize(jsonimage);
        guideImagesJson.push_back(jsonimage);
    }
}

bool ShapeTargetCanvas :: deserialize(ofJson& json) {
    
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
                    guideImages[i].deserialize(imagejson.at(i));
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
