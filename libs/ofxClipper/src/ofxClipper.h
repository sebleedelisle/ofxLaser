// =============================================================================
//
// Copyright (c) 2010-2016 Christopher Baker <http://christopherbaker.net>
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
// -  Contribution 2014 Jakob Schlötter <http://the-man-called-jakob.com>
// updated to Clipper 6.1.3a
//
// =============================================================================


#pragma once


#include <vector>
#include "clipper.hpp"
#include "ofPolyline.h"
#include "ofPath.h"


namespace ofx {


class Clipper: public ofxLaserClipper::Clipper
{
public:
    Clipper();
    ~Clipper();

	std::vector<ofPolyline> getClipped(ofxLaserClipper::ClipType clipType,
									   ofPolyWindingMode subFillType = OF_POLY_WINDING_ODD,
									   ofPolyWindingMode clipFillType = OF_POLY_WINDING_ODD,
									   ofxLaserClipper::cInt scale = DEFAULT_CLIPPER_SCALE);
	
	std::vector<ofPolyline> getClippedPolyTree(ofxLaserClipper::ClipType clipType,
									   ofPolyWindingMode subFillType = OF_POLY_WINDING_ODD,
									   ofPolyWindingMode clipFillType = OF_POLY_WINDING_ODD,
									   ofxLaserClipper::cInt scale = DEFAULT_CLIPPER_SCALE);

    ofRectangle getBounds(ofxLaserClipper::cInt scale = DEFAULT_CLIPPER_SCALE) const;

    bool addRectangle(const ofRectangle& rectangle,
                      ofxLaserClipper::PolyType PolyTyp,
                      bool autoClose = false,
                      ofxLaserClipper::cInt scale = DEFAULT_CLIPPER_SCALE);

    bool addPolyline(const ofPolyline& polyline,
                     ofxLaserClipper::PolyType PolyTyp,
                     bool autoClose = false,
                     ofxLaserClipper::cInt scale = DEFAULT_CLIPPER_SCALE);

    bool addPolylines(const std::vector<ofPolyline>& polylines,
                      ofxLaserClipper::PolyType PolyTyp,
                      bool autoClose = false,
                      ofxLaserClipper::cInt scale = DEFAULT_CLIPPER_SCALE);

    bool addPath(const ofPath& path,
                 ofxLaserClipper::PolyType PolyTyp,
                 bool autoClose = false,
                 ofxLaserClipper::cInt scale = DEFAULT_CLIPPER_SCALE);

    static void close(ofPolyline& polyline);
    static void close(std::vector<ofPolyline>& polylines);

    static bool areAllClosed(const std::vector<ofPolyline>& polylines);

    static ofxLaserClipper::IntPoint toClipper(const ofDefaultVertexType& vertex,
                                          ofxLaserClipper::cInt scale);

    static ofxLaserClipper::IntRect toClipper(const ofRectangle& rectangle,
                                         ofxLaserClipper::cInt scale);

    static ofxLaserClipper::Path toClipper(const ofPolyline& polyline,
                                      ofxLaserClipper::cInt scale);

    static ofxLaserClipper::Paths toClipper(const std::vector<ofPolyline>& polylines,
                                       ofxLaserClipper::cInt scale);

    static ofxLaserClipper::Paths toClipper(const ofPath& path,
                                       ofxLaserClipper::cInt scale);

    static ofRectangle toOf(const ofxLaserClipper::IntRect& rectangle,
                            ofxLaserClipper::cInt scale);

    static ofDefaultVertexType toOf(const ofxLaserClipper::IntPoint& point,
                                    ofxLaserClipper::cInt scale);

    static ofPolyline toOf(const ofxLaserClipper::Path& path,
                           bool isClosed,
                           ofxLaserClipper::cInt scale);

    static std::vector<ofPolyline> toOf(const ofxLaserClipper::Paths& paths,
                                        bool isClosed,
                                        ofxLaserClipper::cInt scale);

    static std::string toString(ofxLaserClipper::ClipType clipType);
    static std::string toString(ofxLaserClipper::PolyType polyType);
    static std::string toString(ofxLaserClipper::PolyFillType polyfillType);
    static std::string toString(ofxLaserClipper::InitOptions initOption);
    static std::string toString(ofxLaserClipper::JoinType joinType);
    static std::string toString(ofxLaserClipper::EndType endType);
    
    static ofPolyWindingMode toOf(ofxLaserClipper::PolyFillType polyfillType);
    static ofxLaserClipper::PolyFillType toClipper(ofPolyWindingMode windingMode);

    static std::vector<ofPolyline> simplifyPolyline(const ofPolyline& polyline,
                                                    ofPolyWindingMode windingMode = OF_POLY_WINDING_ODD,
                                                    ofxLaserClipper::cInt scale = DEFAULT_CLIPPER_SCALE);

    static std::vector<ofPolyline> simplifyPolylines(const std::vector<ofPolyline>& polylines,
                                                     ofPolyWindingMode windingMode = OF_POLY_WINDING_ODD,
                                                     ofxLaserClipper::cInt scale = DEFAULT_CLIPPER_SCALE);

    static std::vector<ofPolyline> simplifyPath(const ofPath& path,
                                                ofPolyWindingMode windingMode = OF_POLY_WINDING_ODD,
                                                ofxLaserClipper::cInt scale = DEFAULT_CLIPPER_SCALE);

    static std::vector<ofPolyline> getOffsets(const std::vector<ofPolyline>& polylines,
                                              double offset,
                                              ofxLaserClipper::JoinType jointype = ofxLaserClipper::jtSquare,
                                              ofxLaserClipper::EndType endtype = ofxLaserClipper::etOpenSquare,
                                              double miterLimit = DEFAULT_MITER_LIMIT,
                                              double arcTolerance = DEFAULT_ARC_TOLERANCE,
                                              ofxLaserClipper::cInt scale = DEFAULT_CLIPPER_SCALE);

    //    // Static Utility Functions
//    ////////////////////////////////////////////////
//    static bool Orientation(const ofPolyline& poly);
//
//    static double Area(const ofPolyline& poly);
//
//    static void OffsetPolylines(const std::vector<ofPolyline>& in_polys,
//
//                                std::vector<ofPolyline>& out_polys,
//
//                                double offset,
//
//                                ClipperLib::JoinType jointype = ClipperLib::jtSquare,
//
//                                ClipperLib::EndType endtype = ClipperLib::etOpenSquare,
//
//                                double MiterLimit = 2);
//
//    static void OffsetPath(ofPath &in_path,
//                                ofPath &out_path,
//                                double offset,
//                                ClipperLib::JoinType jointype = ClipperLib::jtSquare,
//                                ClipperLib::EndType endtype = ClipperLib::etOpenSquare,
//                                double MiterLimit = 2);
//
//    static void SimplifyPolyline(ofPolyline &in_poly,
//                                 std::vector<ofPolyline>& out_polys,
//                                 ofPolyWindingMode windingMode = OF_POLY_WINDING_ODD);


//    static void SimplifyPolylines(const std::vector<ofPolyline>& in_polys,
//                                  std::vector<ofPolyline>& out_polys,
//                                  ofPolyWindingMode windingMode = OF_POLY_WINDING_ODD);
//
//    static void SimplifyPolylines(const std::vector<ofPolyline>& polys,
//                                  ofPolyWindingMode windingMode = OF_POLY_WINDING_ODD);
//
//    static void SimplifyPath(const ofPath &path,
//                             std::vector<ofPolyline>& out_polys,
//                             ofPolyWindingMode windingMode = OF_POLY_WINDING_ODD);
//
//    static void ReversePolyline(ofPolyline& p);
//
//    static void ReversePolylines(std::vector<ofPolyline>& p);
//
//    static void ReversePath(ofPath& p,std::vector<ofPolyline> &out_polys);
//

    static const ofxLaserClipper::cInt DEFAULT_CLIPPER_SCALE;
    static const double DEFAULT_MITER_LIMIT;
    static const double DEFAULT_ARC_TOLERANCE;

private:

};


} // namespace ofx
