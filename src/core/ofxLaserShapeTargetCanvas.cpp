//
//  ofxLaserShapeTargetCanvas.cpp
//
//  Created by Seb Lee-Delisle on 23/02/2023.
//
//

#include "ofxLaserShapeTargetCanvas.h"
#include <algorithm>

using namespace ofxLaser;

ShapeTargetCanvas :: ShapeTargetCanvas() {
    zoneGroup = 0;
    type = ZoneId::CANVAS;
} 

void ShapeTargetCanvas::markSpatialIndexDirty() {
    spatialIndexDirty = true;
}

void ShapeTargetCanvas::deleteShapes() {
    ShapeTarget::deleteShapes();
    markSpatialIndexDirty();
}

bool ShapeTargetCanvas::addShape(std::shared_ptr<Shape> shapetoadd, bool useClipRectangle, ofRectangle clipRectangle) {
    const bool added = ShapeTarget::addShape(shapetoadd, useClipRectangle, clipRectangle);
    if(added) {
        markSpatialIndexDirty();
    }
    return added;
}

int ShapeTargetCanvas::getCellX(float x) const {
    return ofClamp(static_cast<int>((x - spatialIndexedBounds.getLeft()) / spatialCellWidth), 0, kSpatialGridResolution - 1);
}

int ShapeTargetCanvas::getCellY(float y) const {
    return ofClamp(static_cast<int>((y - spatialIndexedBounds.getTop()) / spatialCellHeight), 0, kSpatialGridResolution - 1);
}

void ShapeTargetCanvas::rebuildSpatialIndexIfNeeded() {
    const ofRectangle currentBounds = getBounds();
    if((!spatialIndexDirty) &&
       (spatialIndexedBounds == currentBounds) &&
       (spatialIndexedSourceShapeCount == shapes.size())) {
        return;
    }

    // Full rebuild: clear all indexed state first.
    spatialIndexedBounds = currentBounds;
    spatialIndexedShapes.clear();
    spatialIndexedShapeBounds.clear();
    spatialGridCells.clear();
    spatialGridCells.resize(kSpatialGridResolution * kSpatialGridResolution);
    spatialIndexedSourceShapeCount = shapes.size();

    const float boundsWidth = std::max(1.0f, spatialIndexedBounds.getWidth());
    const float boundsHeight = std::max(1.0f, spatialIndexedBounds.getHeight());
    spatialCellWidth = boundsWidth / static_cast<float>(kSpatialGridResolution);
    spatialCellHeight = boundsHeight / static_cast<float>(kSpatialGridResolution);

    spatialIndexedShapes.reserve(shapes.size());
    spatialIndexedShapeBounds.reserve(shapes.size());

    // Index each non-empty shape by its axis-aligned bounds into overlapping cells.
    for(const std::shared_ptr<Shape>& shape : shapes) {
        if(shape == nullptr || shape->isEmpty()) {
            continue;
        }

        const vector<glm::vec3>& shapePoints = shape->getPoints();
        if(shapePoints.empty()) {
            continue;
        }

        float minX = shapePoints.front().x;
        float maxX = shapePoints.front().x;
        float minY = shapePoints.front().y;
        float maxY = shapePoints.front().y;
        for(const glm::vec3& point : shapePoints) {
            if(point.x < minX) minX = point.x;
            if(point.x > maxX) maxX = point.x;
            if(point.y < minY) minY = point.y;
            if(point.y > maxY) maxY = point.y;
        }

        ofRectangle shapeBounds(minX, minY, maxX - minX, maxY - minY);
        const int shapeIndex = static_cast<int>(spatialIndexedShapes.size());
        spatialIndexedShapes.push_back(shape);
        spatialIndexedShapeBounds.push_back(shapeBounds);

        const int minCellX = getCellX(shapeBounds.getLeft());
        const int maxCellX = getCellX(shapeBounds.getRight());
        const int minCellY = getCellY(shapeBounds.getTop());
        const int maxCellY = getCellY(shapeBounds.getBottom());

        for(int y = minCellY; y <= maxCellY; y++) {
            for(int x = minCellX; x <= maxCellX; x++) {
                spatialGridCells[(y * kSpatialGridResolution) + x].push_back(shapeIndex);
            }
        }
    }

    spatialIndexDirty = false;
}

void ShapeTargetCanvas::gatherCandidateShapeIndices(const ofRectangle& rect, vector<int>& outIndices) {
    outIndices.clear();
    if(spatialIndexedShapes.empty()) {
        return;
    }

    const int minCellX = getCellX(rect.getLeft());
    const int maxCellX = getCellX(rect.getRight());
    const int minCellY = getCellY(rect.getTop());
    const int maxCellY = getCellY(rect.getBottom());

    // Deduplicate candidates gathered from multiple overlapping cells.
    std::unordered_set<int> uniqueCandidates;
    uniqueCandidates.reserve(spatialIndexedShapes.size());
    for(int y = minCellY; y <= maxCellY; y++) {
        for(int x = minCellX; x <= maxCellX; x++) {
            const vector<int>& cell = spatialGridCells[(y * kSpatialGridResolution) + x];
            for(int shapeIndex : cell) {
                uniqueCandidates.insert(shapeIndex);
            }
        }
    }

    outIndices.reserve(uniqueCandidates.size());
    for(int shapeIndex : uniqueCandidates) {
        outIndices.push_back(shapeIndex);
    }
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
        // Build/reuse a spatial index once, then only test likely shapes.
        rebuildSpatialIndexIfNeeded();

        vector<int> candidateShapeIndices;
        gatherCandidateShapeIndices(inputZone->getRect(), candidateShapeIndices);
        newshapes.reserve(candidateShapeIndices.size());

        for(int shapeIndex : candidateShapeIndices) {
            if((shapeIndex < 0) || (static_cast<size_t>(shapeIndex) >= spatialIndexedShapes.size())) {
                continue;
            }
            const std::shared_ptr<Shape>& shape = spatialIndexedShapes[shapeIndex];
            if((shape != nullptr) && (!shape->isEmpty()) && shape->intersectsRect(inputZone->getRect())) {
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
