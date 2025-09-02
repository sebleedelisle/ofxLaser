//
//  ClipperUtils.h
//  Liberation
//
//  Created by Seb Lee-Delisle on 16/10/2023.
//

#pragma once
#include "ofMain.h"
#include "ofxClipper.h"
#include "ofxLaserShape.h"

class ClipperUtils {
  
    public :
    
    // sets up the Z value to store the colours
    static void initialise() {
        clipper.ZFillFunction([](ofxLaserClipper::IntPoint&z1, ofxLaserClipper::IntPoint&z2, ofxLaserClipper::IntPoint& pt){
            pt.Z = z1.Z & 0xffffff;
        });
    }

    static void addShapeToMasks(std::shared_ptr<ofxLaser::Shape> element, ofxLaserClipper::Paths& clipperMasks);
    static void addShapesToMasks(vector<std::shared_ptr<ofxLaser::Shape>> elements, ofxLaserClipper::Paths& clipperMasks);
    static vector<std::shared_ptr<ofxLaser::Shape>> clipShapeToMask(std::shared_ptr<ofxLaser::Shape> shape, ofxLaserClipper::Paths& clipperMasks);
    
    static bool pointWithinMask(glm::vec3 point, ofxLaserClipper::Paths& clipperMasks);
    
    static ofxLaserClipper::Path shapeToClipper(std::shared_ptr<ofxLaser::Shape> shape);

    static std::shared_ptr<ofxLaser::Shape> clipperPathToShape(ofxLaserClipper::Path& path, std::shared_ptr<ofxLaser::Shape> originalShape);
    static vector<std::shared_ptr<ofxLaser::Shape>> clipperPathsToShapes(vector<ofxLaserClipper::Path>& paths, std::shared_ptr<ofxLaser::Shape> originalShape);
    
    
//    static bool isElementInsideElement(BaseGraphicElement* elementToCheck, BaseGraphicElement* element);
//    static  bool isElementInsideElements(BaseGraphicElement* elementToCheck, vector<std::shared_ptr<BaseGraphicElement>>& elements);
//
//    static bool doesElementIntersectElement(BaseGraphicElement* element1, BaseGraphicElement* element2);
//    static bool doesElementIntersectElements(BaseGraphicElement* element1, vector<std::shared_ptr<BaseGraphicElement>>& elements);
//

    
//    static bool pointWithinMask(ElementPoint* element, ClipperLib::Paths& clipperMasks);
//
//
////    static vector<std::shared_ptr<BaseGraphicElement>>  combineElements(vector<std::shared_ptr<BaseGraphicElement>>& elements, ClipperLib::ClipType cliptype );
//
//    static ElementPolyline* polylineToElement(ofPolyline& polyline);

//

//
//    template<class T>// T can be anything with an x and y
//    static bool isPointInsideElement(T& p, BaseGraphicElement* element);
//
    
    static ofx::Clipper clipper;
    
    
};

