// =============================================================================
//
// Copyright (c) 2010-2014 Christopher Baker <http://christopherbaker.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// =============================================================================


#include "ofxClipper.h"


namespace ofx {


	const ofxLaserClipper::cInt Clipper::DEFAULT_CLIPPER_SCALE = 1000; // 1000000;
	const double Clipper::DEFAULT_MITER_LIMIT = 2;
	const double Clipper::DEFAULT_ARC_TOLERANCE = 0.25;


Clipper::Clipper()
{
}


Clipper::~Clipper()
{
}


ofRectangle Clipper::getBounds(ofxLaserClipper::cInt scale) const
{
    return toOf(GetBounds(), scale);
}


std::vector<ofPolyline> Clipper::getClipped(ofxLaserClipper::ClipType clipType,
                                            ofPolyWindingMode subFillType,
                                            ofPolyWindingMode clipFillType,
                                            ofxLaserClipper::cInt scale)
{
    std::vector<ofPolyline> results;

    bool success = false;

    try
    {
        ofxLaserClipper::Paths out;

        bool success = Execute(clipType,
                               out,
                               toClipper(subFillType),
                               toClipper(clipFillType));

        results = toOf(out, true, scale);

        if (!success)
        {
            //ofLogError("Clipper::getClipped") << "Failed to create clipped paths.";
        }
    }
    catch (const std::exception& exc)
    {
        ofLogError("Clipper::getClipped") << exc.what();
    }

    return results;
}


std::vector<ofPolyline> Clipper::getClippedPolyTree(ofxLaserClipper::ClipType clipType,
                                                    ofPolyWindingMode subFillType,
                                                    ofPolyWindingMode clipFillType,
                                                    ofxLaserClipper::cInt scale)
{
    std::vector<ofPolyline> results;
    
    bool success = false;
    
    try
    {
        ofxLaserClipper::PolyTree out;
        
        bool success = Execute(clipType,
                               out,
                               toClipper(subFillType),
                               toClipper(clipFillType));
        
        
        
        if (!success)
        {
            ofLogError("Clipper::getClipped") << "Failed to create clipped paths.";
        } else {
            ofxLaserClipper:: Paths paths;
            OpenPathsFromPolyTree(out, paths);
            results = toOf(paths, false, scale);
        }
    }
    catch (const std::exception& exc)
    {
        ofLogError("Clipper::getClipped") << exc.what();
    }
    
    return results;
}
	

bool Clipper::addRectangle(const ofRectangle& rectangle,
                           ofxLaserClipper::PolyType PolyTyp,
                           bool autoClose,
                           ofxLaserClipper::cInt scale)
{
    return addPolyline(ofPolyline::fromRectangle(rectangle),
                       PolyTyp,
                       autoClose,
                       scale);
}


bool Clipper::addPolyline(const ofPolyline& polyline,
                          ofxLaserClipper::PolyType PolyTyp,
                          bool autoClose,
                          ofxLaserClipper::cInt scale)
{
    auto _polyline = polyline;
    if (autoClose) close(_polyline);
    return AddPath(toClipper(_polyline, scale), PolyTyp, _polyline.isClosed());
}

bool Clipper::addPolylines(const std::vector<ofPolyline>& polylines,
                           ofxLaserClipper::PolyType PolyTyp,
                           bool autoClose,
                           ofxLaserClipper::cInt scale)
{
    auto _polylines = polylines;

    if (autoClose)
    {
        close(_polylines);
    }

    bool isClosed = autoClose ? true : areAllClosed(_polylines);

    return AddPaths(toClipper(_polylines, scale), PolyTyp, isClosed);
}

    
bool Clipper::addPath(const ofPath& paths,
                      ofxLaserClipper::PolyType PolyTyp,
                      bool autoClose,
                      ofxLaserClipper::cInt scale)
{
    return addPolylines(paths.getOutline(), PolyTyp, autoClose, scale);
}


void Clipper::close(ofPolyline& polyline)
{
    polyline.close();
}


void Clipper::close(std::vector<ofPolyline>& polylines)
{
    for (auto& polyline: polylines) close(polyline);
}


bool Clipper::areAllClosed(const std::vector<ofPolyline>& polylines)
{
    for (auto& polyline: polylines) if (!polyline.isClosed()) return false;
    return true;
}


ofxLaserClipper::IntRect Clipper::toClipper(const ofRectangle& rectangle,
                                       ofxLaserClipper::cInt scale)
{
    ofxLaserClipper::IntRect rect;
    rect.left = rectangle.getLeft() * scale;
    rect.right = rectangle.getRight() * scale;
    rect.top = rectangle.getTop() * scale;
    rect.bottom = rectangle.getBottom() * scale;
    return rect;
}


ofxLaserClipper::IntPoint Clipper::toClipper(const ofDefaultVertexType& vertex,
                                        ofxLaserClipper::cInt scale)
{
    return ofxLaserClipper::IntPoint(vertex.x * scale, vertex.y * scale);
}


ofxLaserClipper::Path Clipper::toClipper(const ofPolyline& polyline,
                                    ofxLaserClipper::cInt scale)
{
    ofxLaserClipper::Path path;

    for (auto& vertex: polyline.getVertices())
    {
        path.push_back(toClipper(vertex, scale));
    }

    return path;
}


ofxLaserClipper::Paths Clipper::toClipper(const std::vector<ofPolyline>& polylines,
                                     ofxLaserClipper::cInt scale)
{
    ofxLaserClipper::Paths paths;
    for (auto& polyline: polylines) paths.push_back(toClipper(polyline, scale));
    return paths;
}


ofxLaserClipper::Paths Clipper::toClipper(const ofPath& path, ofxLaserClipper::cInt scale)
{
    return toClipper(path.getOutline(), scale);
}


ofDefaultVertexType Clipper::toOf(const ofxLaserClipper::IntPoint& point,
                                  ofxLaserClipper::cInt scale)
{
    ofDefaultVertexType vertex;
    vertex.x = point.X / scale;
    vertex.y = point.Y / scale;
    return vertex;
}


ofRectangle Clipper::toOf(const ofxLaserClipper::IntRect& rectangle, ofxLaserClipper::cInt scale)
{
    return ofRectangle(rectangle.left / scale,
                       rectangle.top / scale,
                       (rectangle.right - rectangle.left) / scale,
                       (rectangle.bottom - rectangle.top) / scale);
}


ofPolyline Clipper::toOf(const ofxLaserClipper::Path& path,
                         bool isClosed,
                         ofxLaserClipper::cInt scale)
{
    ofPolyline polyline;
    for (auto& point: path) polyline.addVertex(toOf(point, scale));
    if (isClosed) polyline.close();
    return polyline;
}


std::vector<ofPolyline> Clipper::toOf(const ofxLaserClipper::Paths& paths,
                                      bool isClosed,
                                      ofxLaserClipper::cInt scale)
{
    std::vector<ofPolyline> results;
    for (auto& path: paths) results.push_back(toOf(path, isClosed, scale));
    return results;
}


std::string Clipper::toString(ofxLaserClipper::ClipType clipType)
{
    switch (clipType)
    {
        case ofxLaserClipper::ctIntersection: return "ctIntersection";
        case ofxLaserClipper::ctUnion: return "ctUnion";
        case ofxLaserClipper::ctDifference: return "ctDifference";
        case ofxLaserClipper::ctXor: return "ctXor";
        default: ofLogError("Clipper::toOf") << "Unknown clipType.";
    }

    return "UNKNOWN";
}
    
    
std::string Clipper::toString(ofxLaserClipper::PolyType polyType)
{
    switch (polyType)
    {
        case ofxLaserClipper::ptSubject: return "ptSubject";
        case ofxLaserClipper::ptClip: return "ptClip";
        default: ofLogError("Clipper::toOf") << "Unknown polyType.";
    }

    return "UNKNOWN";
}


std::string Clipper::toString(ofxLaserClipper::PolyFillType polyfillType)
{
    switch (polyfillType)
    {
        case ofxLaserClipper::pftEvenOdd: return "pftEvenOdd";
        case ofxLaserClipper::pftNonZero: return "pftNonZero";
        case ofxLaserClipper::pftPositive: return "pftPositive";
        case ofxLaserClipper::pftNegative: return "pftNegative";
        default: ofLogError("Clipper::toOf") << "Unknown polyfillType.";
    }

    return "UNKNOWN";
}


std::string Clipper::toString(ofxLaserClipper::InitOptions initOption)
{
    switch (initOption)
    {
        case ofxLaserClipper::ioReverseSolution: return "ioReverseSolution";
        case ofxLaserClipper::ioStrictlySimple: return "ioStrictlySimple";
        case ofxLaserClipper::ioPreserveCollinear: return "ioPreserveCollinear";
        default: ofLogError("Clipper::toOf") << "Unknown initOption.";
    }

    return "UNKNOWN";
}


std::string Clipper::toString(ofxLaserClipper::JoinType joinType)
{
    switch (joinType)
    {
        case ofxLaserClipper::jtSquare: return "jtSquare";
        case ofxLaserClipper::jtRound: return "jtRound";
        case ofxLaserClipper::jtMiter: return "jtMiter";
        default: ofLogError("Clipper::toOf") << "Unknown joinType.";
    }

    return "UNKNOWN";
}


std::string Clipper::toString(ofxLaserClipper::EndType endType)
{
    switch (endType)
    {
        case ofxLaserClipper::etClosedPolygon: return "etClosedPolygon";
        case ofxLaserClipper::etClosedLine: return "etClosedLine";
        case ofxLaserClipper::etOpenButt: return "etOpenButt";
        case ofxLaserClipper::etOpenSquare: return "etOpenSquare";
        case ofxLaserClipper::etOpenRound: return "etOpenRound";
        default: ofLogError("Clipper::toOf") << "Unknown endType.";
    }

    return "UNKNOWN";
}


ofPolyWindingMode Clipper::toOf(ofxLaserClipper::PolyFillType polyfillType)
{
    switch (polyfillType)
    {
        case ofxLaserClipper::pftEvenOdd: return OF_POLY_WINDING_ODD;
        case ofxLaserClipper::pftNonZero: return OF_POLY_WINDING_NONZERO;
        case ofxLaserClipper::pftPositive: return OF_POLY_WINDING_POSITIVE;
        case ofxLaserClipper::pftNegative: return OF_POLY_WINDING_NEGATIVE;
        default: ofLogError("Clipper::toOf") << "Unknown polyfillType.";
    }

    return OF_POLY_WINDING_ODD;
}



ofxLaserClipper::PolyFillType Clipper::toClipper(ofPolyWindingMode windingMode)
{
    switch (windingMode)
    {
        case OF_POLY_WINDING_ODD: return ofxLaserClipper::pftEvenOdd;
        case OF_POLY_WINDING_NONZERO: return ofxLaserClipper::pftNonZero;
        case OF_POLY_WINDING_POSITIVE: return ofxLaserClipper::pftPositive;
        case OF_POLY_WINDING_NEGATIVE: return ofxLaserClipper::pftNegative;
        case OF_POLY_WINDING_ABS_GEQ_TWO:
        default: ofLogError("Clipper::toClipper") << "Unsupported winding mode, using ClipperLib::pftEvenOdd.";
    }

    return ofxLaserClipper::pftEvenOdd;
}




std::vector<ofPolyline> Clipper::simplifyPolyline(const ofPolyline& polyline,
                                                  ofPolyWindingMode windingMode,
                                                  ofxLaserClipper::cInt scale)
{
    ofxLaserClipper::Paths out;
    ofxLaserClipper::SimplifyPolygon(toClipper(polyline, scale),
                                out,
                                toClipper(windingMode));
    return toOf(out, true, scale);
}

std::vector<ofPolyline> Clipper::simplifyPolylines(const std::vector<ofPolyline>& polylines,
                                                   ofPolyWindingMode windingMode,
                                                   ofxLaserClipper::cInt scale)
{
    ofxLaserClipper::Paths out;
    ofxLaserClipper::SimplifyPolygons(toClipper(polylines, scale),
                                 out,
                                 toClipper(windingMode));
    return toOf(out, true, scale);
}

std::vector<ofPolyline> Clipper::simplifyPath(const ofPath& path,
                                              ofPolyWindingMode windingMode,
                                              ofxLaserClipper::cInt scale)
{
    ofxLaserClipper::Paths out;
    ofxLaserClipper::SimplifyPolygons(toClipper(path, scale),
                                 out,
                                 toClipper(windingMode));

    return toOf(out, true, scale);
}


std::vector<ofPolyline> Clipper::getOffsets(const std::vector<ofPolyline>& polylines,
                                            double offset,
                                            ofxLaserClipper::JoinType jointype,
                                            ofxLaserClipper::EndType endtype,
                                            double miterLimit,
                                            double arcTolerance,
                                            ofxLaserClipper::cInt scale)
{
    offset *= scale;
    miterLimit *= scale;
    arcTolerance *= scale;

    ofxLaserClipper::Paths out;

    ofxLaserClipper::ClipperOffset clipperOffset(miterLimit, arcTolerance);
    clipperOffset.AddPaths(toClipper(polylines, scale), jointype, endtype);
    clipperOffset.Execute(out, offset);

    return toOf(out, true, scale);
}


} // namespace ofx
