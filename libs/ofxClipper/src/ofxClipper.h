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


class Clipper: public ClipperLib::Clipper
{
public:
    Clipper();
    ~Clipper();

	std::vector<ofPolyline> getClipped(ClipperLib::ClipType clipType,
									   ofPolyWindingMode subFillType = OF_POLY_WINDING_ODD,
									   ofPolyWindingMode clipFillType = OF_POLY_WINDING_ODD,
									   ClipperLib::cInt scale = DEFAULT_CLIPPER_SCALE);
	
	std::vector<ofPolyline> getClippedPolyTree(ClipperLib::ClipType clipType,
									   ofPolyWindingMode subFillType = OF_POLY_WINDING_ODD,
									   ofPolyWindingMode clipFillType = OF_POLY_WINDING_ODD,
									   ClipperLib::cInt scale = DEFAULT_CLIPPER_SCALE);

    ofRectangle getBounds(ClipperLib::cInt scale = DEFAULT_CLIPPER_SCALE) const;

    bool addRectangle(const ofRectangle& rectangle,
                      ClipperLib::PolyType PolyTyp,
                      bool autoClose = false,
                      ClipperLib::cInt scale = DEFAULT_CLIPPER_SCALE);

    bool addPolyline(const ofPolyline& polyline,
                     ClipperLib::PolyType PolyTyp,
                     bool autoClose = false,
                     ClipperLib::cInt scale = DEFAULT_CLIPPER_SCALE);

    bool addPolylines(const std::vector<ofPolyline>& polylines,
                      ClipperLib::PolyType PolyTyp,
                      bool autoClose = false,
                      ClipperLib::cInt scale = DEFAULT_CLIPPER_SCALE);

    bool addPath(const ofPath& path,
                 ClipperLib::PolyType PolyTyp,
                 bool autoClose = false,
                 ClipperLib::cInt scale = DEFAULT_CLIPPER_SCALE);

    static void close(ofPolyline& polyline);
    static void close(std::vector<ofPolyline>& polylines);

    static bool areAllClosed(const std::vector<ofPolyline>& polylines);

    static ClipperLib::IntPoint toClipper(const ofDefaultVertexType& vertex,
                                          ClipperLib::cInt scale);

    static ClipperLib::IntRect toClipper(const ofRectangle& rectangle,
                                         ClipperLib::cInt scale);

    static ClipperLib::Path toClipper(const ofPolyline& polyline,
                                      ClipperLib::cInt scale);

    static ClipperLib::Paths toClipper(const std::vector<ofPolyline>& polylines,
                                       ClipperLib::cInt scale);

    static ClipperLib::Paths toClipper(const ofPath& path,
                                       ClipperLib::cInt scale);

    static ofRectangle toOf(const ClipperLib::IntRect& rectangle,
                            ClipperLib::cInt scale);

    static ofDefaultVertexType toOf(const ClipperLib::IntPoint& point,
                                    ClipperLib::cInt scale);

    static ofPolyline toOf(const ClipperLib::Path& path,
                           bool isClosed,
                           ClipperLib::cInt scale);

    static std::vector<ofPolyline> toOf(const ClipperLib::Paths& paths,
                                        bool isClosed,
                                        ClipperLib::cInt scale);

    static std::string toString(ClipperLib::ClipType clipType);
    static std::string toString(ClipperLib::PolyType polyType);
    static std::string toString(ClipperLib::PolyFillType polyfillType);
    static std::string toString(ClipperLib::InitOptions initOption);
    static std::string toString(ClipperLib::JoinType joinType);
    static std::string toString(ClipperLib::EndType endType);
    
    static ofPolyWindingMode toOf(ClipperLib::PolyFillType polyfillType);
    static ClipperLib::PolyFillType toClipper(ofPolyWindingMode windingMode);

    static std::vector<ofPolyline> simplifyPolyline(const ofPolyline& polyline,
                                                    ofPolyWindingMode windingMode = OF_POLY_WINDING_ODD,
                                                    ClipperLib::cInt scale = DEFAULT_CLIPPER_SCALE);

    static std::vector<ofPolyline> simplifyPolylines(const std::vector<ofPolyline>& polylines,
                                                     ofPolyWindingMode windingMode = OF_POLY_WINDING_ODD,
                                                     ClipperLib::cInt scale = DEFAULT_CLIPPER_SCALE);

    static std::vector<ofPolyline> simplifyPath(const ofPath& path,
                                                ofPolyWindingMode windingMode = OF_POLY_WINDING_ODD,
                                                ClipperLib::cInt scale = DEFAULT_CLIPPER_SCALE);

    static std::vector<ofPolyline> getOffsets(const std::vector<ofPolyline>& polylines,
                                              double offset,
                                              ClipperLib::JoinType jointype = ClipperLib::jtSquare,
                                              ClipperLib::EndType endtype = ClipperLib::etOpenSquare,
                                              double miterLimit = DEFAULT_MITER_LIMIT,
                                              double arcTolerance = DEFAULT_ARC_TOLERANCE,
                                              ClipperLib::cInt scale = DEFAULT_CLIPPER_SCALE);

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

    static const ClipperLib::cInt DEFAULT_CLIPPER_SCALE;
    static const double DEFAULT_MITER_LIMIT;
    static const double DEFAULT_ARC_TOLERANCE;

private:

};


} // namespace ofx
