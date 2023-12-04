//
//  ClipperUtils.cpp
//  Liberation
//
//  Created by Seb Lee-Delisle on 16/10/2023.
//

#include <ClipperUtils.h>




ofx::Clipper ClipperUtils::clipper;

void ClipperUtils :: addShapeToMasks(ofxLaser::Shape* element, ClipperLib::Paths& clipperMasks){

    clipper.Clear();
    try {

        ClipperLib::Path elementMask = shapeToClipper(element);
        //ClipperLib::Paths elementMasks = {elementMask}; // elementToClipper(element);

//
//        if(element->mask==BaseGraphicElement::MASK_STATE_OUTSIDE) {
//            ClipperLib::Path rectpath;
//            rectpath.push_back(ClipperLib::IntPoint(-1000000, -1000000));
//            rectpath.push_back(ClipperLib::IntPoint(1000000, -1000000));
//            rectpath.push_back(ClipperLib::IntPoint(1000000, 1000000));
//            rectpath.push_back(ClipperLib::IntPoint(-1000000, 1000000));
//            clipper.AddPath(rectpath, ClipperLib::ptSubject, true);
//            clipper.AddPath(elementMask, ClipperLib::ptSubject, true);
//            clipper.Execute(ClipperLib::ctDifference, elementMasks, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);
//
//
//        }

        if(clipperMasks.size()==0) {
            clipperMasks = {elementMask}; // .push_back(elementMask);
            return;
        }
        clipper.AddPath(elementMask, ClipperLib::ptSubject, true);
        clipper.AddPaths(clipperMasks, ClipperLib::ptClip, true);

        //vector<ofPolyline> targetPieces = clipper.getClipped(ClipperLib::ctUnion);
        clipper.Execute(ClipperLib::ctUnion, clipperMasks, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);

        //masks = combinedElements;
       // return clippedElements;


    } catch(...) {
        ofLogError("Clipper error!");
        //return ; // vector<BaseGraphicElement*> { element};
    }


}



void ClipperUtils :: addShapesToMasks(vector<ofxLaser::Shape*> elements, ClipperLib::Paths& clipperMasks){

    if(elements.size()>1) {
       // ofLogNotice("ClipperUtils :: addShapesToMasks");
    }
    
    
    clipper.Clear();
    try {

        ClipperLib::Paths clipperElements;
        for(ofxLaser::Shape* element : elements) {
            if(!element->isEmpty()) {
                clipperElements.push_back(shapeToClipper(element));
            }
        }
        
        if(clipperElements.size()==0) return; 
        //ClipperLib::Path elementMask = shapeToClipper(element);


        if(clipperMasks.size()==0) {
            clipperMasks = clipperElements;
            return;
        }
        clipper.AddPaths(clipperElements, ClipperLib::ptSubject, true);
        clipper.AddPaths(clipperMasks, ClipperLib::ptClip, true);

        //vector<ofPolyline> targetPieces = clipper.getClipped(ClipperLib::ctUnion);
        clipper.Execute(ClipperLib::ctUnion, clipperMasks, ClipperLib::pftEvenOdd, ClipperLib::pftEvenOdd);

        //masks = combinedElements;
       // return clippedElements;


    } catch(...) {
        ofLogError("Clipper error!");
        //return ; // vector<BaseGraphicElement*> { element};
    }


}


vector<ofxLaser::Shape*> ClipperUtils :: clipShapeToMask(ofxLaser::Shape* shape, ClipperLib::Paths& clipperMasks) {
    
    if(shape->isEmpty()) {
        return {};
    }

    if(clipperMasks.size()==0) {
        return { shape->clone() };
    }

    // IF IT'S a POINT :
   
    if(shape->getPoints().size()==1) {

        if(pointWithinMask(shape->getStartPos(), clipperMasks)) {
            return { } ;
        } else {
            return {shape->clone() };
        }

    }
    // otherwise ...

    vector<ofxLaser :: Shape*> clippedElements;

    clipper.Clear();

    try {
        //clipper.addPolyline(elementToPolyline(element), ClipperLib::ptSubject, false);
        ClipperLib::Path elementpath = shapeToClipper(shape);
        clipper.AddPath(elementpath, ClipperLib::ptSubject, false);

        clipper.AddPaths(clipperMasks, ClipperLib::ptClip, true);

        try {
            ClipperLib::PolyTree out;

            bool success = clipper.Execute(ClipperLib::ctDifference,
                                out,
                                clipper.toClipper(OF_POLY_WINDING_ODD),
                                clipper.toClipper(OF_POLY_WINDING_ODD));



            if (!success) {
                ofLogError("Clipper::getClipped") << "Failed to create clipped paths.";
            } else {
                ClipperLib:: Paths paths;
                OpenPathsFromPolyTree(out, paths);
                clippedElements = clipperPathsToShapes(paths, shape );
            }

        } catch (const std::exception& exc) {

            ofLogError("Clipper::getClipped") << exc.what();

        }

        
        
        return clippedElements;


    } catch(...) {
        ofLogError("Clipper error!");

        return vector<ofxLaser::Shape*> { };
    }

}

bool ClipperUtils ::  pointWithinMask(glm::vec3 vertex, ClipperLib::Paths& clipperMasks){

    clipper.Clear();
    
    ClipperLib::IntPoint p = ClipperLib::IntPoint(vertex.x * ofx::Clipper::DEFAULT_CLIPPER_SCALE, vertex.y * ofx::Clipper::DEFAULT_CLIPPER_SCALE);

    for(ClipperLib::Path& path : clipperMasks)  {
        if(PointInPolygon(p, path)!=0) return true;
    }
    return false;

}


// TODO needs optimisation
ClipperLib::Path ClipperUtils :: shapeToClipper(ofxLaser::Shape* shape) {

    // TODO check shape is closed
    
    ClipperLib::Path path;

    ClipperLib::IntPoint p(0,0, 0xffffff);

    // not sure why we're going backwards, don't think it makes a difference?
    //for(int i = shape->getNumPoints()-1; i>=0; i--) {
    //    glm::vec3 vertex = shape->getPointAt(i);
    vector<glm::vec3>& points = shape->getPoints();
    int numpoints = shape->isClosed()? points.size()+1 : points.size();
    for(int i = 0; i<numpoints; i++)  {
        glm::vec3& vertex = points.at(i%points.size());
        
        ofColor c = shape->getColourAtPoint(i%points.size());
        long long hexvalue = c.getHex();
        
        // storing a bit code !
        if(i==0) hexvalue |= 0x01000000; // bit 25 for start point
        else if(i==numpoints-1) hexvalue |= 0x02000000; // bit 26 for end point
        
        p = ClipperLib::IntPoint(vertex.x * ofx::Clipper::DEFAULT_CLIPPER_SCALE, vertex.y * ofx::Clipper::DEFAULT_CLIPPER_SCALE, hexvalue);

       path.push_back(p);
    }
    
    // open closed paths
    if(path.size()>2) {
        if(path.front()==path.back()) {
            path.back().X-=1;
        }
        
//
//        glm::vec3&front = points.front();
//        glm::vec3&back = points.back();
//        float distance = glm::distance(front, back);
//        // if the shape is closed then open it
//        if(distance < 0.0001f) {
//            // fairly complex code to disconnect the ends of the shape
//
//            glm::vec3& vertex1 = points[0];
//            glm::vec3& vertex2 = points[1];
//
//            float length = glm::distance(vertex1, vertex2);
//
//            float trimScalar = 0.1f/length;
//            vertex1 = glm::mix(vertex1,vertex2, trimScalar);
//
//            path.front().X = vertex1.x * ofx::Clipper::DEFAULT_CLIPPER_SCALE;
//            path.front().Y = vertex1.y * ofx::Clipper::DEFAULT_CLIPPER_SCALE;
//
//        }
        
    }


    
//
//    if(dynamic_cast<ElementPoint*>(element)) {
//        p.X+=2;
//        path.push_back(p);
//    }

    return path;
}



ofxLaser :: Shape* ClipperUtils :: clipperPathToShape(ClipperLib::Path& path, ofxLaser :: Shape* originalElement) {

    ofxLaser :: Shape* newelement = originalElement->clone();
    newelement->clear();
    
    
    if(!originalElement->isMultiColoured()) {
        newelement->setColour(originalElement->getColour());
    }

    bool pathclosed = (path.front()==path.back());
    int numpoints = pathclosed?path.size()-1 : path.size();
    
    for(int i = 0; i<numpoints; i++) {
        ClipperLib :: IntPoint cp = path.at(i);
        glm::vec3 p ((float)cp.X/ofx::Clipper::DEFAULT_CLIPPER_SCALE, (float)cp.Y/ofx::Clipper::DEFAULT_CLIPPER_SCALE, 0);
        ofFloatColor c = ofColor::fromHex(cp.Z);
        if(originalElement->isMultiColoured()) {
            newelement->addPoint(p, c);
        } else {
            newelement->addPoint(p);
        } 


    }
    
    newelement->setClosed(pathclosed);
    
    return newelement;
}


vector<ofxLaser :: Shape*> ClipperUtils :: clipperPathsToShapes(vector<ClipperLib::Path>& paths, ofxLaser :: Shape* originalElement) {
    if(paths.size()==0) return {};
    
        int startIndex = -1;
        int endIndex = -1;
    
    if((paths.size()>1) && originalElement->isClosed()) {
        bool startAtBack = true;
        bool endAtFront = true;
        
        for(int i = 0; i<paths.size(); i++ ) {
            ClipperLib::Path& path = paths.at(i);
            
            if(path.front().Z & 0x01000000) {
                //ofLogNotice("found start at front of : ") << i;
                startIndex = i;
                startAtBack = false;
            } else if(path.front().Z & 0x02000000) {
                //ofLogNotice("found end at front of: ") << i;
                endIndex = i;
                endAtFront = true;
            }
            if(path.back().Z & 0x01000000) {
                //ofLogNotice("found start at back of : ") << i;
                startIndex = i;
                startAtBack = true;
            } else if(path.back().Z & 0x02000000) {
                //ofLogNotice("found end at back of : ") << i;
                endIndex = i;
                endAtFront = false;
            }
        }
        
        if((startIndex>=0) && (endIndex>=0)) {
            ClipperLib::Path& startPath = paths.at(startIndex);
            ClipperLib::Path& endPath = paths.at(endIndex);
            
            // I think that the start is always at the back
            // and the end is always at the front but we'll see I guess!
            if(!startAtBack) {
                // reverse
                std::reverse(startPath.begin(), startPath.end());
                
            }
            if(!endAtFront) {
                //reverse it
                std::reverse(endPath.begin(), endPath.end());
            }
            
            ClipperLib::Path newPath;
            newPath.insert(newPath.end(), startPath.begin(), startPath.end());
            if(endPath.size()>1) {
                newPath.insert(newPath.end(), endPath.begin()+1, endPath.end());
            }
            
            
//            for(auto& p : newPath) {
//                p.Z = 0xffffff;
//            }
            paths.push_back(newPath);
        }
            
            
        
        
    }
    
    vector<ofxLaser :: Shape*> newelements;
    for(int i = 0; i<paths.size(); i++) {
        if((i==startIndex) || (i==endIndex)) continue;
        ClipperLib::Path& path = paths.at(i);
        newelements.push_back(clipperPathToShape(path, originalElement));
    }

    return newelements;
}

//
//bool ClipperUtils::isElementInsideElement(BaseGraphicElement* elementToCheck, BaseGraphicElement* element) {
//
//    // if any of the points are outside the element then we're not enclosed
//    vector<glm::vec3>& points1 = elementToCheck->getPoints();
//    for(glm::vec3& p : points1) {
//        if(!isPointInsideElement(p, element)) return false;
//    }
//    // and if any of the containing element points are inside the element
//    // to check then we're not enclosed
//    vector<glm::vec3>& points2 = element->getPoints();
//    for(glm::vec3& p : points2) {
//        if(isPointInsideElement(p, elementToCheck)) return false;
//    }
//
//
//    return true;
//}
//
//bool ClipperUtils::isElementInsideElements(BaseGraphicElement* elementToCheck, vector<BaseGraphicElement*>& maskPolylines) {
//    for(BaseGraphicElement* maskpoly : maskPolylines) {
//        if(isElementInsideElement(elementToCheck, maskpoly)) return true;
//    }
//    return false;
//}
//
//bool ClipperUtils::doesElementIntersectElement(BaseGraphicElement* element1, BaseGraphicElement* element2){
//
//    glm::vec3 p1 = element1->getPointAt(0);
//    glm::vec3 p2;
//
//    glm::vec3 p3, p4, p5;
//
//    int  n1 = element1->getNumPoints();
//    int  n2 = element2->getNumPoints();
//
//    for(int i = 1; i<=n1; i++) {
//
//        p2 = element1->getPointAt(i % n1);
//
//        p3 = element2->getPointAt(0);
//        for(int j = 1; j<=n2; j++) {
//            p4 = element2->getPointAt(j%n2);
//
//            if(ofLineSegmentIntersection(p1, p2, p3, p4, p5)) return true;
//
//            p3 = p4;
//        }
//        p1 = p2;
//    }
//
//
//    return false;
//
//}
//
//bool ClipperUtils::doesElementIntersectElements(BaseGraphicElement* element1, vector<BaseGraphicElement*>& elements) {
//    for(BaseGraphicElement* element : elements) {
//        if(doesElementIntersectElement(element1, element)) return true;
//    }
//    return false;
//}
//
//
////
////vector<BaseGraphicElement*> Patch::subtractElements(BaseGraphicElement* elementToSubtract, BaseGraphicElement* element) {
////
////    vector<BaseGraphicElement*> returnelements;
////
////    clipper.Clear();
////    try {
////        clipper.addPolyline(elementToPolyline(element), ClipperLib::ptSubject, true);
////
////        clipper.addPolyline(elementToPolyline(elementToSubtract), ClipperLib::ptClip, true);
////
////        vector<ofPolyline> targetPieces = clipper.getClipped(ClipperLib::ctDifference);
////        ofSetLineWidth(10);
////
////        elementToPolyline(element).draw();
////        elementToPolyline(elementToSubtract).draw();
////
////        for(ofPolyline& poly : targetPieces) {
////
////            ofSetColor(ofColor::red);
////            poly.draw();
////        }
////
////        return returnelements;
////
////
////    } catch(...) {
////
////        ofLogError("Clipper error!");
////
////        return returnelements;
////       //return vector<BaseGraphicElement*> { element};
////    }
////
////
////}
//


////
////vector<BaseGraphicElement*> ClipperUtils :: combineElements(vector<BaseGraphicElement*>& elements, ClipperLib::ClipType cliptype ) {
////
////    if(elements.size()<2) {
////        return {};
////    }
////
////    //ClipperLib::Paths combinedPaths;
////
////    for(int i = 0; i<elements.size(); i++) {
////        BaseGraphicElement* element = elements[i];
////
////        ClipperLib::Path elementpath = ClipperUtils :: elementToClipper(element);
////        if(i<elements.size()-1) {
////            ClipperUtils :: clipper.AddPath(elementpath, ClipperLib::ptSubject, false);
////        } else {
////
////            ClipperUtils :: clipper.AddPath(elementpath, ClipperLib::ptClip, true);
////        }
////
////    }
////
////    ClipperLib::PolyTree out;
////    ClipperLib:: Paths paths;
////    bool success = clipper.Execute(cliptype,
////                        out,
////                        clipper.toClipper(OF_POLY_WINDING_ODD),
////                        clipper.toClipper(OF_POLY_WINDING_ODD));
////
////
////    OpenPathsFromPolyTree(out, paths);
////    vector<BaseGraphicElement*> newelements = clipperPathsToElements(paths, elements[0]);
////
////    return newelements;
////}
//
//

//
//template<class T>
//bool ClipperUtils::isPointInsideElement(T& p, BaseGraphicElement* element) {
//
//    int counter = 0;
//    int i;
//    double xinters;
//    glm::vec3 p1,p2;
//
//    int N = element->getNumPoints();
//
//    p1 = element->getPointAt(0);
//    for (i=1;i<=N;i++) {
//        p2 = element->getPointAt(i % N);
//        if (p.y > std::min(p1.y,p2.y)) {
//            if (p.y <= std::max(p1.y,p2.y)) {
//                if (p.x <= std::max(p1.x,p2.x)) {
//                    if (p1.y != p2.y) {
//                        xinters = (p.y-p1.y)*(p2.x-p1.x)/(p2.y-p1.y)+p1.x;
//                        if (p1.x == p2.x || p.x <= xinters)
//                            counter++;
//                    }
//                }
//            }
//        }
//        p1 = p2;
//    }
//
//    if (counter % 2 == 0) return false;
//    else return true;
//
//}
//
