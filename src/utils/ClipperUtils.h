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
        clipper.ZFillFunction([](ClipperLib::IntPoint&z1, ClipperLib::IntPoint&z2, ClipperLib::IntPoint& pt){
            pt.Z = z1.Z & 0xffffff;
        });
    }

    static void addShapeToMasks(ofxLaser::Shape* element, ClipperLib::Paths& clipperMasks);
   
    static vector<ofxLaser::Shape*> clipShapeToMask(ofxLaser::Shape* shape, ClipperLib::Paths& clipperMasks);
    
    static bool pointWithinMask(glm::vec3 point, ClipperLib::Paths& clipperMasks);
    
    static ClipperLib::Path shapeToClipper(ofxLaser::Shape* shape);

    static ofxLaser :: Shape* clipperPathToShape(ClipperLib::Path& path, ofxLaser :: Shape* originalShape);
    static vector<ofxLaser :: Shape*> clipperPathsToShapes(vector<ClipperLib::Path>& paths, ofxLaser::Shape* originalShape);
    
    
//    static bool isElementInsideElement(BaseGraphicElement* elementToCheck, BaseGraphicElement* element);
//    static  bool isElementInsideElements(BaseGraphicElement* elementToCheck, vector<BaseGraphicElement*>& elements);
//
//    static bool doesElementIntersectElement(BaseGraphicElement* element1, BaseGraphicElement* element2);
//    static bool doesElementIntersectElements(BaseGraphicElement* element1, vector<BaseGraphicElement*>& elements);
//

    
//    static bool pointWithinMask(ElementPoint* element, ClipperLib::Paths& clipperMasks);
//
//
////    static vector<BaseGraphicElement*>  combineElements(vector<BaseGraphicElement*>& elements, ClipperLib::ClipType cliptype );
//
//    static ElementPolyline* polylineToElement(ofPolyline& polyline);

//

//
//    template<class T>// T can be anything with an x and y
//    static bool isPointInsideElement(T& p, BaseGraphicElement* element);
//
    
    static ofx::Clipper clipper;
    
    
};

