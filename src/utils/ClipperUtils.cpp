//
//  ClipperUtils.cpp
//  Liberation
//
//  Created by Seb Lee-Delisle on 16/10/2023.
//

#include <ClipperUtils.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>




ofx::Clipper ClipperUtils::clipper;

namespace {

constexpr float clipIntervalEpsilon = 1e-6f;
constexpr float pointMergeEpsilon = 5e-4f;

struct SegmentInterval {
    float startT = 0.0f;
    float endT = 0.0f;
};

struct ShapePieceData {
    vector<glm::vec3> points;
    vector<ofFloatColor> colours;
};

bool pointsAreNear(const glm::vec3& a, const glm::vec3& b) {
    return glm::distance2(a, b) <= (pointMergeEpsilon * pointMergeEpsilon);
}

float pointToSegmentT(const glm::vec3& p0, const glm::vec3& p1, const glm::vec3& point) {
    const glm::vec3 direction = p1 - p0;
    const float denominator = glm::dot(direction, direction);
    if(denominator <= std::numeric_limits<float>::epsilon()) {
        return 0.0f;
    }
    return glm::dot(point - p0, direction) / denominator;
}

void addUniqueTValue(vector<float>& tValues, float t) {
    t = ofClamp(t, 0.0f, 1.0f);
    for(const float existing : tValues) {
        if(std::abs(existing - t) <= clipIntervalEpsilon) {
            return;
        }
    }
    tValues.push_back(t);
}

vector<vector<glm::vec3>> convertMasksToFloatPolygons(const ofxLaserClipper::Paths& clipperMasks) {
    vector<vector<glm::vec3>> maskPolygons;
    maskPolygons.reserve(clipperMasks.size());

    const float invScale = 1.0f / static_cast<float>(ofx::Clipper::DEFAULT_CLIPPER_SCALE);

    for(const ofxLaserClipper::Path& path : clipperMasks) {
        if(path.size() < 2) {
            continue;
        }

        vector<glm::vec3> polygon;
        polygon.reserve(path.size());
        for(const ofxLaserClipper::IntPoint& p : path) {
            polygon.emplace_back(static_cast<float>(p.X) * invScale, static_cast<float>(p.Y) * invScale, 0.0f);
        }

        if(polygon.size() > 2 && pointsAreNear(polygon.front(), polygon.back())) {
            polygon.pop_back();
        }
        if(polygon.size() >= 2) {
            maskPolygons.push_back(std::move(polygon));
        }
    }

    return maskPolygons;
}

vector<SegmentInterval> subtractMasksFromSegment(const glm::vec3& p0,
                                                 const glm::vec3& p1,
                                                 const vector<vector<glm::vec3>>& maskPolygons,
                                                 const ofxLaserClipper::Paths& clipperMasks) {
    if(pointsAreNear(p0, p1)) {
        return {};
    }

    vector<float> tValues = {0.0f, 1.0f};
    for(const vector<glm::vec3>& polygon : maskPolygons) {
        if(polygon.size() < 2) {
            continue;
        }

        for(size_t i = 0; i < polygon.size(); ++i) {
            const glm::vec3& edgeStart = polygon[i];
            const glm::vec3& edgeEnd = polygon[(i + 1) % polygon.size()];

            glm::vec3 intersection;
            if(ofLineSegmentIntersection(p0, p1, edgeStart, edgeEnd, intersection)) {
                addUniqueTValue(tValues, pointToSegmentT(p0, p1, intersection));
            }
        }
    }

    std::sort(tValues.begin(), tValues.end());

    vector<SegmentInterval> outsideIntervals;
    outsideIntervals.reserve(tValues.size());

    for(size_t i = 1; i < tValues.size(); ++i) {
        const float t0 = tValues[i - 1];
        const float t1 = tValues[i];
        if((t1 - t0) <= clipIntervalEpsilon) {
            continue;
        }

        const float midpointT = (t0 + t1) * 0.5f;
        const glm::vec3 midpoint = glm::mix(p0, p1, midpointT);
        if(!ClipperUtils::pointWithinMask(midpoint, clipperMasks)) {
            outsideIntervals.push_back({t0, t1});
        }
    }

    return outsideIntervals;
}

void appendPointToPiece(ShapePieceData& piece,
                        const glm::vec3& point,
                        const ofFloatColor& colour,
                        bool multiColoured) {
    if(piece.points.empty() || !pointsAreNear(piece.points.back(), point)) {
        piece.points.push_back(point);
        if(multiColoured) {
            piece.colours.push_back(colour);
        }
        return;
    }

    if(multiColoured && !piece.colours.empty()) {
        piece.colours.back() = colour;
    }
}

void flushPieceIfValid(ShapePieceData& currentPiece, vector<ShapePieceData>& pieces) {
    if(currentPiece.points.size() >= 2) {
        pieces.push_back(std::move(currentPiece));
    }
    currentPiece = ShapePieceData{};
}

} // namespace

void ClipperUtils :: addShapeToMasks(std::shared_ptr<ofxLaser::Shape> element, ofxLaserClipper::Paths& clipperMasks){

    clipper.Clear();
    try {

        ofxLaserClipper::Path elementMask = shapeToClipper(element);
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
        clipper.AddPath(elementMask, ofxLaserClipper::ptSubject, true);
        clipper.AddPaths(clipperMasks, ofxLaserClipper::ptClip, true);

        //vector<ofPolyline> targetPieces = clipper.getClipped(ClipperLib::ctUnion);
        clipper.Execute(ofxLaserClipper::ctUnion, clipperMasks, ofxLaserClipper::pftEvenOdd, ofxLaserClipper::pftEvenOdd);

        //masks = combinedElements;
       // return clippedElements;


    } catch(...) {
        ofLogError("Clipper error!");
        //return ; // vector<std::shared_ptr<BaseGraphicElement>> { element};
    }


}



void ClipperUtils :: addShapesToMasks(vector<std::shared_ptr<ofxLaser::Shape>> elements, ofxLaserClipper::Paths& clipperMasks){

    if(elements.size()>1) {
       // ofLogNotice("ClipperUtils :: addShapesToMasks");
    }
    
    
    clipper.Clear();
    try {

        ofxLaserClipper::Paths clipperElements;
        clipperElements.reserve(elements.size());
        for(std::shared_ptr<ofxLaser::Shape>& element : elements) {
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
        clipper.AddPaths(clipperElements, ofxLaserClipper::ptSubject, true);
        clipper.AddPaths(clipperMasks, ofxLaserClipper::ptClip, true);

        //vector<ofPolyline> targetPieces = clipper.getClipped(ClipperLib::ctUnion);
        clipper.Execute(ofxLaserClipper::ctUnion, clipperMasks, ofxLaserClipper::pftEvenOdd, ofxLaserClipper::pftEvenOdd);

        //masks = combinedElements;
       // return clippedElements;


    } catch(...) {
        ofLogError("Clipper error!");
        //return ; // vector<std::shared_ptr<BaseGraphicElement>> { element};
    }


}


vector<std::shared_ptr<ofxLaser::Shape>> ClipperUtils :: clipShapeToMask(std::shared_ptr<ofxLaser::Shape> shape, ofxLaserClipper::Paths& clipperMasks) {
    if(shape->isEmpty()) {
        return {};
    }

    if(clipperMasks.empty()) {
        return { shape->clone() };
    }

    // IF IT'S a POINT :
    if(shape->getPoints().size() == 1) {

        if(pointWithinMask(shape->getStartPos(), clipperMasks)) {
            return { } ;
        } else {
            return {shape->clone() };
        }

    }
    // otherwise ...
    const vector<glm::vec3>& shapePoints = shape->getPoints();
    if(shapePoints.size() < 2) {
        return {};
    }

    const bool closed = shape->isClosed();
    const bool multiColoured = shape->isMultiColoured();
    const size_t pointCount = shapePoints.size();
    const size_t segmentCount = closed ? pointCount : pointCount - 1;

    const vector<vector<glm::vec3>> maskPolygons = convertMasksToFloatPolygons(clipperMasks);

    vector<ShapePieceData> pieceData;
    pieceData.reserve(segmentCount);
    ShapePieceData currentPiece;
    if(multiColoured) {
        currentPiece.colours.reserve(pointCount);
    }

    for(size_t segmentIndex = 0; segmentIndex < segmentCount; ++segmentIndex) {
        const glm::vec3& p0 = shapePoints[segmentIndex];
        const glm::vec3& p1 = shapePoints[(segmentIndex + 1) % pointCount];

        const vector<SegmentInterval> outsideIntervals = subtractMasksFromSegment(p0, p1, maskPolygons, clipperMasks);

        if(outsideIntervals.empty()) {
            flushPieceIfValid(currentPiece, pieceData);
            continue;
        }

        if(outsideIntervals.front().startT > clipIntervalEpsilon) {
            flushPieceIfValid(currentPiece, pieceData);
        }

        for(size_t intervalIndex = 0; intervalIndex < outsideIntervals.size(); ++intervalIndex) {
            const SegmentInterval& interval = outsideIntervals[intervalIndex];
            const glm::vec3 startPoint = glm::mix(p0, p1, interval.startT);
            const glm::vec3 endPoint = glm::mix(p0, p1, interval.endT);

            const float startIndex = static_cast<float>(segmentIndex) + interval.startT;
            const float endIndex = static_cast<float>(segmentIndex) + interval.endT;
            const ofFloatColor startColour = shape->getColourAtFloatIndex(startIndex);
            const ofFloatColor endColour = shape->getColourAtFloatIndex(endIndex);

            appendPointToPiece(currentPiece, startPoint, startColour, multiColoured);
            appendPointToPiece(currentPiece, endPoint, endColour, multiColoured);

            const bool hasGapAfter = (intervalIndex + 1 < outsideIntervals.size()) &&
                                     ((outsideIntervals[intervalIndex + 1].startT - interval.endT) > clipIntervalEpsilon);
            if(hasGapAfter) {
                flushPieceIfValid(currentPiece, pieceData);
            }
        }

        if(outsideIntervals.back().endT < (1.0f - clipIntervalEpsilon)) {
            flushPieceIfValid(currentPiece, pieceData);
        }
    }

    flushPieceIfValid(currentPiece, pieceData);

    // Closed strokes are cyclic, so if we split at the arbitrary index-0 seam
    // we can safely stitch back when first/last piece meet there.
    if(closed && pieceData.size() > 1) {
        ShapePieceData& firstPiece = pieceData.front();
        ShapePieceData& lastPiece = pieceData.back();
        if(!firstPiece.points.empty() && !lastPiece.points.empty() &&
           pointsAreNear(lastPiece.points.back(), firstPiece.points.front())) {
            ShapePieceData mergedPiece;
            mergedPiece.points.reserve(lastPiece.points.size() + firstPiece.points.size() - 1);
            mergedPiece.points.insert(mergedPiece.points.end(), lastPiece.points.begin(), lastPiece.points.end());
            mergedPiece.points.insert(mergedPiece.points.end(), firstPiece.points.begin() + 1, firstPiece.points.end());

            if(multiColoured) {
                mergedPiece.colours.reserve(lastPiece.colours.size() + firstPiece.colours.size() - 1);
                mergedPiece.colours.insert(mergedPiece.colours.end(), lastPiece.colours.begin(), lastPiece.colours.end());
                if(firstPiece.colours.size() > 1) {
                    mergedPiece.colours.insert(mergedPiece.colours.end(), firstPiece.colours.begin() + 1, firstPiece.colours.end());
                }
            }

            firstPiece = std::move(mergedPiece);
            pieceData.pop_back();
        }
    }

    vector<std::shared_ptr<ofxLaser::Shape>> clippedElements;
    clippedElements.reserve(pieceData.size());

    for(ShapePieceData& piece : pieceData) {
        bool pieceClosed = closed &&
                           (piece.points.size() > 2) &&
                           pointsAreNear(piece.points.front(), piece.points.back());
        if(pieceClosed) {
            piece.points.pop_back();
            if(multiColoured && !piece.colours.empty()) {
                piece.colours.pop_back();
            }
        }

        if(piece.points.size() < 2) {
            continue;
        }

        std::shared_ptr<ofxLaser::Shape> newShape = shape->clone();
        newShape->setPoints(piece.points, pieceClosed);
        if(multiColoured) {
            newShape->setColours(piece.colours);
        } else {
            newShape->setColour(shape->getColour());
        }
        newShape->setClipRectangle(shape->getClipRectangle());
        clippedElements.push_back(newShape);
    }

    return clippedElements;
}

bool ClipperUtils ::  pointWithinMask(glm::vec3 vertex, const ofxLaserClipper::Paths& clipperMasks){

    bool inside = false;
    const auto scale = ofx::Clipper::DEFAULT_CLIPPER_SCALE;

    const ofxLaserClipper::IntPoint p(vertex.x * scale, vertex.y * scale);

    for(const ofxLaserClipper::Path& path : clipperMasks)  {
        if(PointInPolygon(p, path)!=0) inside = !inside;
    }
    return inside;

}


// TODO needs optimisation
ofxLaserClipper::Path ClipperUtils :: shapeToClipper(std::shared_ptr<ofxLaser::Shape> shape) {

    // TODO check shape is closed
    
    ofxLaserClipper::Path path;
    vector<glm::vec3>& points = shape->getPoints();
    if(points.empty()) {
        return path;
    }

    const auto scale = ofx::Clipper::DEFAULT_CLIPPER_SCALE;
    const size_t pointCount = points.size();
    const size_t numpoints = shape->isClosed() ? pointCount + 1 : pointCount;
    path.reserve(numpoints);

    for(size_t i = 0; i<numpoints; i++)  {
        const size_t pointIndex = (i < pointCount) ? i : 0;
        const glm::vec3& vertex = points[pointIndex];

        long long hexvalue = shape->getColourAtPoint(static_cast<int>(pointIndex)).getHex();

        // storing a bit code !
        if(i==0) hexvalue |= 0x01000000; // bit 25 for start point
        else if(i==numpoints-1) hexvalue |= 0x02000000; // bit 26 for end point

        path.emplace_back(vertex.x * scale, vertex.y * scale, hexvalue);
    }
    
    // open closed paths
    if(path.size()>2) {
        // OK SO THIS SOMETIMES CAUSES PROBLEMS ***** KEEP AN EYE ON IT
        if(path.front()==path.back()) {
            path.back().X+=1;
            path.back().Y-=1;
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



std::shared_ptr<ofxLaser::Shape> ClipperUtils :: clipperPathToShape(ofxLaserClipper::Path& path, std::shared_ptr<ofxLaser::Shape> originalElement) {

    std::shared_ptr<ofxLaser::Shape> newelement = originalElement->clone();

    bool pathclosed = (path.front()==path.back());
    const int numpoints = pathclosed ? static_cast<int>(path.size()) - 1 : static_cast<int>(path.size());
    const bool multiColoured = originalElement->isMultiColoured();
    const float invScale = 1.0f / static_cast<float>(ofx::Clipper::DEFAULT_CLIPPER_SCALE);

    vector<glm::vec3> points;
    points.reserve(numpoints);
    vector<ofFloatColor> colours;
    if(multiColoured) {
        colours.reserve(numpoints);
    }

    for(int i = 0; i<numpoints; i++) {
        const ofxLaserClipper::IntPoint& cp = path[i];
        points.emplace_back(static_cast<float>(cp.X) * invScale, static_cast<float>(cp.Y) * invScale, 0);
        if(multiColoured) {
            colours.emplace_back(ofColor::fromHex(cp.Z));
        }
    }

    newelement->setPoints(points, pathclosed);
    if(multiColoured) {
        newelement->setColours(colours);
    } else {
        newelement->setColour(originalElement->getColour());
    }
    newelement->setClipRectangle(originalElement->getClipRectangle());
    
    return newelement;
}


vector<std::shared_ptr<ofxLaser::Shape>> ClipperUtils :: clipperPathsToShapes(vector<ofxLaserClipper::Path>& paths, std::shared_ptr<ofxLaser::Shape> originalElement) {
    if(paths.empty()) return {};
    
        int startIndex = -1;
        int endIndex = -1;
    
    if((paths.size()>1) && originalElement->isClosed()) {
        bool startAtBack = true;
        bool endAtFront = true;
        
        for(int i = 0; i<paths.size(); i++ ) {
            ofxLaserClipper::Path& path = paths.at(i);
            
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
            ofxLaserClipper::Path& startPath = paths.at(startIndex);
            ofxLaserClipper::Path& endPath = paths.at(endIndex);
            
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
            
            ofxLaserClipper::Path newPath;
            newPath.reserve(startPath.size() + (endPath.size()>1 ? endPath.size()-1 : 0));
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
    
    vector<std::shared_ptr<ofxLaser::Shape>> newelements;
    newelements.reserve(paths.size());
    for(int i = 0; i<paths.size(); i++) {
        if((i==startIndex) || (i==endIndex)) continue;
        ofxLaserClipper::Path& path = paths.at(i);
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
//bool ClipperUtils::isElementInsideElements(BaseGraphicElement* elementToCheck, vector<std::shared_ptr<BaseGraphicElement>>& maskPolylines) {
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
//bool ClipperUtils::doesElementIntersectElements(BaseGraphicElement* element1, vector<std::shared_ptr<BaseGraphicElement>>& elements) {
//    for(std::shared_ptr<BaseGraphicElement>& element : elements) {
//        if(doesElementIntersectElement(element1, element)) return true;
//    }
//    return false;
//}
//
//
////
////vector<std::shared_ptr<BaseGraphicElement>> Patch::subtractElements(BaseGraphicElement* elementToSubtract, BaseGraphicElement* element) {
////
////    vector<std::shared_ptr<BaseGraphicElement>> returnelements;
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
////       //return vector<std::shared_ptr<BaseGraphicElement>> { element};
////    }
////
////
////}
//


////
////vector<std::shared_ptr<BaseGraphicElement>> ClipperUtils :: combineElements(vector<std::shared_ptr<BaseGraphicElement>>& elements, ClipperLib::ClipType cliptype ) {
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
////    vector<std::shared_ptr<BaseGraphicElement>> newelements = clipperPathsToElements(paths, elements[0]);
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
