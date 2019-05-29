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


#include "ofx/Clipper.h"


namespace ofx {


const ClipperLib::cInt Clipper::DEFAULT_CLIPPER_SCALE = 1000000;
const double Clipper::DEFAULT_MITER_LIMIT = 2;
const double Clipper::DEFAULT_ARC_TOLERANCE = 0.25;


Clipper::Clipper()
{
}


Clipper::~Clipper()
{
}


ofRectangle Clipper::getBounds(ClipperLib::cInt scale) const
{
    return toOf(GetBounds(), scale);
}


std::vector<ofPolyline> Clipper::getClipped(ClipperLib::ClipType clipType,
                                            ofPolyWindingMode subFillType,
                                            ofPolyWindingMode clipFillType,
                                            ClipperLib::cInt scale)
{
    std::vector<ofPolyline> results;

    bool success = false;

    try
    {
        ClipperLib::Paths out;

        bool success = Execute(clipType,
                               out,
                               toClipper(subFillType),
                               toClipper(clipFillType));

        results = toOf(out, true, scale);

        if (!success)
        {
            ofLogError("Clipper::getClipped") << "Failed to create clipped paths.";
        }
    }
    catch (const std::exception& exc)
    {
        ofLogError("Clipper::getClipped") << exc.what();
    }

    return results;
}

	
std::vector<ofPolyline> Clipper::getClippedPolyTree(ClipperLib::ClipType clipType,
												ofPolyWindingMode subFillType,
												ofPolyWindingMode clipFillType,
												ClipperLib::cInt scale)
	{
		std::vector<ofPolyline> results;
		
		bool success = false;
		
		try
		{
			ClipperLib::PolyTree out;
			
			bool success = Execute(clipType,
								   out,
								   toClipper(subFillType),
								   toClipper(clipFillType));
			
			
			
			if (!success)
			{
				ofLogError("Clipper::getClipped") << "Failed to create clipped paths.";
			} else {
				ClipperLib:: Paths paths;
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
                           ClipperLib::PolyType PolyTyp,
                           bool autoClose,
                           ClipperLib::cInt scale)
{
    return addPolyline(ofPolyline::fromRectangle(rectangle),
                       PolyTyp,
                       autoClose,
                       scale);
}


bool Clipper::addPolyline(const ofPolyline& polyline,
                          ClipperLib::PolyType PolyTyp,
                          bool autoClose,
                          ClipperLib::cInt scale)
{
    auto _polyline = polyline;
    if (autoClose) close(_polyline);
    return AddPath(toClipper(_polyline, scale), PolyTyp, _polyline.isClosed());
}

bool Clipper::addPolylines(const std::vector<ofPolyline>& polylines,
                           ClipperLib::PolyType PolyTyp,
                           bool autoClose,
                           ClipperLib::cInt scale)
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
                      ClipperLib::PolyType PolyTyp,
                      bool autoClose,
                      ClipperLib::cInt scale)
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


ClipperLib::IntRect Clipper::toClipper(const ofRectangle& rectangle,
                                       ClipperLib::cInt scale)
{
    ClipperLib::IntRect rect;
    rect.left = rectangle.getLeft() * scale;
    rect.right = rectangle.getRight() * scale;
    rect.top = rectangle.getTop() * scale;
    rect.bottom = rectangle.getBottom() * scale;
    return rect;
}


ClipperLib::IntPoint Clipper::toClipper(const ofDefaultVertexType& vertex,
                                        ClipperLib::cInt scale)
{
    return ClipperLib::IntPoint(vertex.x * scale, vertex.y * scale);
}


ClipperLib::Path Clipper::toClipper(const ofPolyline& polyline,
                                    ClipperLib::cInt scale)
{
    ClipperLib::Path path;

    for (auto& vertex: polyline.getVertices())
    {
        path.push_back(toClipper(vertex, scale));
    }

    return path;
}


ClipperLib::Paths Clipper::toClipper(const std::vector<ofPolyline>& polylines,
                                     ClipperLib::cInt scale)
{
    ClipperLib::Paths paths;
    for (auto& polyline: polylines) paths.push_back(toClipper(polyline, scale));
    return paths;
}


ClipperLib::Paths Clipper::toClipper(const ofPath& path, ClipperLib::cInt scale)
{
    return toClipper(path.getOutline(), scale);
}


ofDefaultVertexType Clipper::toOf(const ClipperLib::IntPoint& point,
                                  ClipperLib::cInt scale)
{
    ofDefaultVertexType vertex;
    vertex.x = point.X / scale;
    vertex.y = point.Y / scale;
    return vertex;
}


ofRectangle Clipper::toOf(const ClipperLib::IntRect& rectangle, ClipperLib::cInt scale)
{
    return ofRectangle(rectangle.left / scale,
                       rectangle.top / scale,
                       (rectangle.right - rectangle.left) / scale,
                       (rectangle.bottom - rectangle.top) / scale);
}


ofPolyline Clipper::toOf(const ClipperLib::Path& path,
                         bool isClosed,
                         ClipperLib::cInt scale)
{
    ofPolyline polyline;
    for (auto& point: path) polyline.addVertex(toOf(point, scale));
    if (isClosed) polyline.close();
    return polyline;
}


std::vector<ofPolyline> Clipper::toOf(const ClipperLib::Paths& paths,
                                      bool isClosed,
                                      ClipperLib::cInt scale)
{
    std::vector<ofPolyline> results;
    for (auto& path: paths) results.push_back(toOf(path, isClosed, scale));
    return results;
}


std::string Clipper::toString(ClipperLib::ClipType clipType)
{
    switch (clipType)
    {
        case ClipperLib::ctIntersection: return "ctIntersection";
        case ClipperLib::ctUnion: return "ctUnion";
        case ClipperLib::ctDifference: return "ctDifference";
        case ClipperLib::ctXor: return "ctXor";
        default: ofLogError("Clipper::toOf") << "Unknown clipType.";
    }

    return "UNKNOWN";
}
    
    
std::string Clipper::toString(ClipperLib::PolyType polyType)
{
    switch (polyType)
    {
        case ClipperLib::ptSubject: return "ptSubject";
        case ClipperLib::ptClip: return "ptClip";
        default: ofLogError("Clipper::toOf") << "Unknown polyType.";
    }

    return "UNKNOWN";
}


std::string Clipper::toString(ClipperLib::PolyFillType polyfillType)
{
    switch (polyfillType)
    {
        case ClipperLib::pftEvenOdd: return "pftEvenOdd";
        case ClipperLib::pftNonZero: return "pftNonZero";
        case ClipperLib::pftPositive: return "pftPositive";
        case ClipperLib::pftNegative: return "pftNegative";
        default: ofLogError("Clipper::toOf") << "Unknown polyfillType.";
    }

    return "UNKNOWN";
}


std::string Clipper::toString(ClipperLib::InitOptions initOption)
{
    switch (initOption)
    {
        case ClipperLib::ioReverseSolution: return "ioReverseSolution";
        case ClipperLib::ioStrictlySimple: return "ioStrictlySimple";
        case ClipperLib::ioPreserveCollinear: return "ioPreserveCollinear";
        default: ofLogError("Clipper::toOf") << "Unknown initOption.";
    }

    return "UNKNOWN";
}


std::string Clipper::toString(ClipperLib::JoinType joinType)
{
    switch (joinType)
    {
        case ClipperLib::jtSquare: return "jtSquare";
        case ClipperLib::jtRound: return "jtRound";
        case ClipperLib::jtMiter: return "jtMiter";
        default: ofLogError("Clipper::toOf") << "Unknown joinType.";
    }

    return "UNKNOWN";
}


std::string Clipper::toString(ClipperLib::EndType endType)
{
    switch (endType)
    {
        case ClipperLib::etClosedPolygon: return "etClosedPolygon";
        case ClipperLib::etClosedLine: return "etClosedLine";
        case ClipperLib::etOpenButt: return "etOpenButt";
        case ClipperLib::etOpenSquare: return "etOpenSquare";
        case ClipperLib::etOpenRound: return "etOpenRound";
        default: ofLogError("Clipper::toOf") << "Unknown endType.";
    }

    return "UNKNOWN";
}


ofPolyWindingMode Clipper::toOf(ClipperLib::PolyFillType polyfillType)
{
    switch (polyfillType)
    {
        case ClipperLib::pftEvenOdd: return OF_POLY_WINDING_ODD;
        case ClipperLib::pftNonZero: return OF_POLY_WINDING_NONZERO;
        case ClipperLib::pftPositive: return OF_POLY_WINDING_POSITIVE;
        case ClipperLib::pftNegative: return OF_POLY_WINDING_NEGATIVE;
        default: ofLogError("Clipper::toOf") << "Unknown polyfillType.";
    }

    return OF_POLY_WINDING_ODD;
}



ClipperLib::PolyFillType Clipper::toClipper(ofPolyWindingMode windingMode)
{
    switch (windingMode)
    {
        case OF_POLY_WINDING_ODD: return ClipperLib::pftEvenOdd;
        case OF_POLY_WINDING_NONZERO: return ClipperLib::pftNonZero;
        case OF_POLY_WINDING_POSITIVE: return ClipperLib::pftPositive;
        case OF_POLY_WINDING_NEGATIVE: return ClipperLib::pftNegative;
        case OF_POLY_WINDING_ABS_GEQ_TWO:
        default: ofLogError("Clipper::toClipper") << "Unsupported winding mode, using ClipperLib::pftEvenOdd.";
    }

    return ClipperLib::pftEvenOdd;
}




std::vector<ofPolyline> Clipper::simplifyPolyline(const ofPolyline& polyline,
                                                  ofPolyWindingMode windingMode,
                                                  ClipperLib::cInt scale)
{
    ClipperLib::Paths out;
    ClipperLib::SimplifyPolygon(toClipper(polyline, scale),
                                out,
                                toClipper(windingMode));
    return toOf(out, true, scale);
}

std::vector<ofPolyline> Clipper::simplifyPolylines(const std::vector<ofPolyline>& polylines,
                                                   ofPolyWindingMode windingMode,
                                                   ClipperLib::cInt scale)
{
    ClipperLib::Paths out;
    ClipperLib::SimplifyPolygons(toClipper(polylines, scale),
                                 out,
                                 toClipper(windingMode));
    return toOf(out, true, scale);
}

std::vector<ofPolyline> Clipper::simplifyPath(const ofPath& path,
                                              ofPolyWindingMode windingMode,
                                              ClipperLib::cInt scale)
{
    ClipperLib::Paths out;
    ClipperLib::SimplifyPolygons(toClipper(path, scale),
                                 out,
                                 toClipper(windingMode));

    return toOf(out, true, scale);
}


std::vector<ofPolyline> Clipper::getOffsets(const std::vector<ofPolyline>& polylines,
                                            double offset,
                                            ClipperLib::JoinType jointype,
                                            ClipperLib::EndType endtype,
                                            double miterLimit,
                                            double arcTolerance,
                                            ClipperLib::cInt scale)
{
    offset *= scale;
    miterLimit *= scale;
    arcTolerance *= scale;

    ClipperLib::Paths out;

    ClipperLib::ClipperOffset clipperOffset(miterLimit, arcTolerance);
    clipperOffset.AddPaths(toClipper(polylines, scale), jointype, endtype);
    clipperOffset.Execute(out, offset);

    return toOf(out, true, scale);
}


} // namespace ofx
