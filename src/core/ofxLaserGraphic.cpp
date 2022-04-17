//
//  ofxLaser :: Graphic
//
//  Created by Seb Lee-Delisle on 01/03/2018.
//
//

#include "ofxLaserGraphic.h"
using namespace ofxLaser;

// static class members
int Graphic::numGraphicsInMemory = 0;


void Graphic :: addSvgFromFile(string filename, bool optimise, bool subtractFills) {
    filename = ofToDataPath(filename);

    ofBuffer buffer = ofBufferFromFile(filename);
    
    addSvgFromString(buffer.getText(), optimise, subtractFills);
	
}

void Graphic :: addSvgFromString(string data, bool optimise, bool subtractFills) {

    ofxSVGExtra svg;
    svg.loadFromString(data);
    addSvg(svg, optimise, subtractFills);
}

void Graphic :: addSvg(ofxSVGExtra& svg, bool optimise, bool subtractFills) {
    
    const vector <ofPath> & paths = svg.getPaths();
    
    for (ofPath path : svg.getPaths()){
        addPath(path, false, subtractFills, true);
    }
    
    if(optimise) {
        connectLineSegments();
        // if we subtracted fills then the lines were optimised already
        if(!subtractFills) {
            for(size_t i= 0; i<polylines.size(); i++) {
                polylines[i]->simplify(0.2);
            }
        }
    }
    
    
}
void Graphic::subtractPathFromPolylines(ofPath& sourcepath) {
	
	if(polylines.size()==0) return;
	vector <ofPolyline*> newPolylines;
	vector <ofColor> newColours;
	
	for(size_t i= 0; i<polylines.size(); i++) {
		
		clipper.Clear();
		
		ofPolyline& target = *polylines[i];
		
		// Add the clipper subjects (i.e. the things that will be clipped).
		clipper.addPolyline(target, ClipperLib::ptSubject);
		
		// add the clipper masks (i.e. the things that will do the clipping).
		clipper.addPath(sourcepath, ClipperLib::ptClip, true); // third param autocloses
		
		// Execute the clipping operation based on the current clipping type.
		vector<ofPolyline> targetPieces = clipper.getClippedPolyTree(ClipperLib::ctDifference);
		for(ofPolyline& poly : targetPieces) {
			poly.simplify();
			newPolylines.push_back(Factory::getPolyline(&poly));
			//newPolylines.back()->simplify();
			newColours.push_back(colours[i]);
		}
		
	}
	
	replacePolylines(newPolylines, newColours);
	
	
}


void Graphic::intersectRect(ofRectangle& rect) {
	
	if(polylines.size()==0) return;
	vector <ofPolyline*> newPolylines;
	vector <ofColor> newColours;
	
	ofPath sourcepath;
	sourcepath.rectangle(rect);
	
	for(size_t i= 0; i<polylines.size(); i++) {
		
		clipper.Clear();
		
		ofPolyline& target = *polylines[i];
		
		try {
			
			// Add the clipper subjects (i.e. the things that will be clipped).
			clipper.addPolyline(target, ClipperLib::ptSubject);
		} catch (...) {
			continue;
		}
		// add the clipper masks (i.e. the things that will do the clipping).
		clipper.addPath(sourcepath, ClipperLib::ptClip, true); // third param autocloses
		
		// Execute the clipping operation based on the current clipping type.
		vector<ofPolyline> targetPieces = clipper.getClippedPolyTree(ClipperLib::ctIntersection);
		for(ofPolyline& poly : targetPieces) {
			poly.simplify();
			newPolylines.push_back(Factory::getPolyline(&poly));
			//newPolylines.back()->simplify();
			newColours.push_back(colours[i]);
		}
		
	}
	
	replacePolylines(newPolylines, newColours);
	
	
}

void Graphic::intersectPaths(vector<ofPath>& paths) {
	
	if(polylines.size()==0) return;
	vector <ofPolyline*> newPolylines;
	vector <ofColor> newColours;
	
	//ofPath sourcepath;
	//sourcepath.rectangle(rect);
	
	for(size_t i= 0; i<polylines.size(); i++) {
		
		clipper.Clear();
		
		ofPolyline& target = *polylines[i];
		
		// Add the clipper subjects (i.e. the things that will be clipped).
		clipper.addPolyline(target, ClipperLib::ptSubject);
		
		// add the clipper masks (i.e. the things that will do the clipping).
		for(ofPath & path : paths) {
			clipper.addPath(path, ClipperLib::ptClip, true); // third param autocloses
		}
		// Execute the clipping operation based on the current clipping type.
		vector<ofPolyline> targetPieces = clipper.getClippedPolyTree(ClipperLib::ctIntersection);
		for(ofPolyline& poly : targetPieces) {
			poly.simplify();
			newPolylines.push_back(Factory::getPolyline(&poly));
			//newPolylines.back()->simplify();
			newColours.push_back(colours[i]);
		}
		
	}
	
	replacePolylines(newPolylines, newColours);
	
	
}



void Graphic :: replacePolylines(vector<ofPolyline*>& newPolylines, vector<ofColor>&newColours){
	// delete all the polylines and colours!
	clear();
	// and now add the updated ones :
	for(size_t i= 0; i<newPolylines.size(); i++) {
	
		ofPolyline* poly = newPolylines[i];
		poly->simplify();
		// get rid of zero length polys
		if((poly->size()<=2) && (poly->getPerimeter()==0)) {
			Factory::releasePolyline(poly);
		} else {
			polylines.push_back(poly);
			colours.push_back(newColours[i]);
			breakPolyline(poly);
		}
	}
	
	
}

bool Graphic:: pointInsidePath(glm::vec3 point, ofPath& path) {

	bool isinside = false;
	for(ofPolyline poly : path.getOutline()) {
		if(poly.inside(point)) {
			isinside = !isinside;
		}
		
	}
	return isinside;
	
}


void Graphic :: translate(glm::vec3 offset) {
	
	for(size_t i= 0; i<polylines.size(); i++) {
		
		ofPolyline& poly =*polylines[i];
		poly.translate(offset);
		
	}
	
}

void Graphic :: autoCentre() {
	
	ofRectangle boundingBox;
	
	for(size_t i= 0; i<polylines.size(); i++) {
		
		ofPolyline& poly =*polylines[i];
		
		if(i==0) {
			boundingBox = poly.getBoundingBox();
		} else {
			boundingBox.growToInclude(poly.getBoundingBox());
		}
		
	}
	
	translate(-boundingBox.getCenter());
}

void Graphic :: addPath(const ofPath& path, bool useTransform, bool subtractFills, bool optimise) {
	// tests for empty paths
	
	ofPath newpath = path;
	
	newpath.setPolyWindingMode(OF_POLY_WINDING_ODD);
	// how do we check for empty?
	if(newpath.getOutline().size()==0) {
		return;
	}
	
	if(useTransform) {
		newpath = transformPath(newpath);
	}
	if(subtractFills && newpath.isFilled()){
		
		subtractPathFromPolylines(newpath);
	}

	if(newpath.hasOutline()){
		
		const vector<ofPolyline> & outlines = newpath.getOutline();
		ofColor col(newpath.getStrokeColor());
		for(const ofPolyline& line:outlines) {
            if(optimise) {
                ofPolyline polyline = line;
                polyline.simplify(0.2);
                addPolyline(polyline, col, false, false);
            } else {
                addPolyline(line, col, false, false);
            }
		}
		
	}
	
}

void Graphic :: addPolyline(const ofPolyline& poly, ofColor colour, bool filled, bool useTransform) {
	
	// we don't need no one point vertices!
	if(poly.size()<2) {
		//ofLog(OF_LOG_NOTICE, "deleting single vertex line");
		return;
	}
	if(poly.getPerimeter()==0) {
		//ofLog(OF_LOG_NOTICE, "deleting empty line");
		return;
	}
	
	ofPolyline* newPoly = Factory::getPolyline(&poly); // make a copy;
	
	if(useTransform) {
		transformPolyline(*newPoly);
	}
	
	
	if(filled) {
		subtractPolyline(newPoly);
		
		clipper.Clear();
		
		clipper.addPolylines(polylineMask,ClipperLib::ptSubject, true);
		clipper.addPolyline(*newPoly, ClipperLib::ptClip, true);
		polylineMask = clipper.getClipped(ClipperLib::ctUnion);
	}
	
	breakPolyline(newPoly);
	newPoly->simplify();
	
	polylines.push_back(newPoly);
	colours.push_back(colour);
	
	
}

void Graphic::breakPolyline(ofPolyline* newPoly) {
//	ofLog(OF_LOG_NOTICE, " --------------------------------");
//	for(glm::vec3 v : newPoly->getVertices())  {
//		ofLog(OF_LOG_NOTICE, ofToString(v));
//	}
//
	// we need polys to be open lines, so that we subtract portions of line, rather than a cohesive shape
	if(newPoly->isClosed()) {
		newPoly->setClosed(false);
		// if we add a point exactly at the start position, Clipper intelligently assumes
		// it's closed... if we make the last point a tiny bit offset, it correctly
		// maintains its openness.
		const vector<glm::vec3>& vertices = newPoly->getVertices();
		
		if(glm::distance(vertices.front(),vertices.back())>1) {
			newPoly->addVertex(vertices.front());
		}
	}
	const vector<glm::vec3>& vertices = newPoly->getVertices();
	
	// if there's already a point exactly at the start position, just shift it a little
	if(glm::distance(vertices.front(),vertices.back())<0.001) {
		vector<glm::vec3>& vertices = newPoly->getVertices(); // not const any more
		// vector between the last two points
		glm::vec3 v = vertices.back()-vertices[vertices.size()-2];
		
		// get parrallel vector that is 0.001 long
		v = glm::normalize(v);
		v *= 0.001;
		// and subtract it from the last point
		vertices.back()-=v; // (vertices.back()+v);
		
		if(isnan(v.x)){
			ofLog(OF_LOG_NOTICE,ofToString(v));
		}
	}
	
}

void Graphic::subtractPolyline(ofPolyline* polyToSubtract, bool useTransform) {

	
	vector <ofPolyline*> newPolylines;
	vector <ofColor> newColours;

	ofPolyline* newPoly = Factory::getPolyline(polyToSubtract); // make a copy;

	if(useTransform) {
		transformPolyline(*newPoly);
	}

	for(size_t i= 0; i<polylines.size(); i++) {
		
		clipper.Clear();

		
		ofPolyline& target = *polylines[i];
		
		// Add the clipper subjects (i.e. the things that will be clipped).
		try {
			clipper.addPolyline(target, ClipperLib::ptSubject);
		}
		catch(...) {
			return;
		}
		// add the clipper masks (i.e. the things that will do the clipping).
		clipper.addPolyline(*newPoly, ClipperLib::ptClip, true); // third param autocloses

		// Execute the clipping operation based on the current clipping type.
		vector<ofPolyline> targetPieces = clipper.getClippedPolyTree(ClipperLib::ctDifference);
		for(ofPolyline& poly : targetPieces) {
			newPolylines.push_back(Factory::getPolyline(&poly));
			newPolylines.back()->simplify();
			newColours.push_back(colours[i]);
		}
		
	}
	
	
	replacePolylines(newPolylines, newColours);
	Factory::releasePolyline(newPoly);

}

void Graphic :: transformPolyline(ofPolyline& poly) {
	vector<glm::vec3>& newPolyVerts = poly.getVertices();

	for(size_t i= 0; i<newPolyVerts.size(); i++) {
		newPolyVerts[i]=gLProject(newPolyVerts[i]);
	}
	
}


ofPath Graphic :: transformPath(ofPath& path) {
	ofPath returnpath = path;
	returnpath.clear();
	for(const ofPolyline& poly : path.getOutline()) {
		ofPolyline newpoly = poly;
		transformPolyline(newpoly);
		returnpath.moveTo(newpoly[0]);
		for( size_t i= 1; i < newpoly.size(); ++i ) returnpath.lineTo( newpoly[(int)i] );
	}
	return returnpath;
	
}
glm::vec3 Graphic::gLProject(glm::vec3& v) {

	ofRectangle rViewport = ofGetCurrentViewport();
	
	glm::mat4 modelview, projection;
	glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(modelview));
	glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(projection));
	glm::mat4 mat = ofGetCurrentOrientationMatrix();
	mat = glm::inverse(mat);
	mat *=projection * modelview;
	glm::vec4 dScreen4 = mat * glm::vec4(v.x,v.y,v.z,1.0);
	glm::vec3 dScreen = glm::vec3(dScreen4) / dScreen4.w;
	dScreen += glm::vec3(1.0) ;
	dScreen *= 0.5;
	
	dScreen.x *= rViewport.width;
	dScreen.x += rViewport.x;
	
	dScreen.y *= rViewport.height;
	dScreen.y += rViewport.y;
	
	return ofPoint(dScreen.x, dScreen.y, 0.0f);
}

void Graphic ::  connectLineSegments() {
	
	for(size_t i= 0; i<polylines.size(); i++) {
		ofPolyline* poly1 = polylines[i];
		
		float smallestAngle = 360;
		int closestIndex = -1;
		
		for(int j = (int)i+1; j<polylines.size(); j++) {
			
			if(colours[i]!=colours[j]) continue; // check close?
			
			ofPolyline* poly2 = polylines[j];
			
			// if polys are the same colour and
			// they are touching, this returns the angle
			// between them, otherwise it returns 360
			
			float angle = comparePolylines(*poly1, *poly2);
			
			if(angle<smallestAngle) {
				smallestAngle = angle;
				closestIndex = j;
			}
			
			
			
		}
		if(closestIndex>=0) {
			ofPolyline* poly2 = polylines[closestIndex];
			//ofLog(OF_LOG_NOTICE, "found touching lines");
			//ofLog(OF_LOG_NOTICE, ofToString(ofRadToDeg(comparePolylines(poly1, poly2))));
			if(joinPolylines(*poly1, *polylines[closestIndex])){
				
//				colours[i] = ofColor::cyan;
//				colours[closestIndex] = ofColor::red;
//				poly2.translate(glm::vec3(10,0,0));
				
				
				// if we erase poly2 is that OK? It'll definitely be>i, right?
				ofPolyline* polyToErase = *(polylines.begin() + closestIndex);
				polylines.erase(polylines.begin() + closestIndex);
				colours.erase(colours.begin() + closestIndex);
				Factory::releasePolyline(polyToErase);
				
				// so we test the newly joined poly again
				i--;
			}
		}
		
		
	}
	
	
}
bool Graphic :: joinPolylines(ofPolyline& poly1, ofPolyline &poly2) {
	
	const vector<glm::vec3>& vertices1 = poly1.getVertices();
	const vector<glm::vec3>& vertices2 = poly2.getVertices();
	
	const glm::vec3& start1 = vertices1.front();
	const glm::vec3& end1 = vertices1.back();
	const glm::vec3& start2 = vertices2.front();
	const glm::vec3& end2 = vertices2.back();
	
	float tolerance = 0.00001f;
	ofPoint p1, p2, p3;
	
	float returnAngle = 360;
	
	bool startClosest1 = false;
	bool startClosest2 = false;
	
	if(glm::distance(end1, start2)<tolerance) {
		
		
		p1 = vertices1[vertices1.size()-2];
		p2 = vertices1.back();
		p3 = vertices2[1];
		
		float angle = getAngleBetweenPoints(p1, p2, p3);
		if(angle<returnAngle) {
			returnAngle =angle;
			startClosest1 = false;
			startClosest2 = true;
		}
	}
	if(glm::distance(start1,start2)<tolerance) {
		
		
		p1 = vertices1[1];
		p2 = vertices1[0];
		p3 = vertices2[1];
		
		float angle = getAngleBetweenPoints(p1, p2, p3);
		if(angle<returnAngle) {
			returnAngle =angle;
			startClosest1 = true;
			startClosest2 = true;
		}
		
	}
	if(glm::distance(end1,end2)<tolerance) {
		
		
		p1 = vertices1[vertices1.size()-2];
		p2 = vertices1.back();
		p3 = vertices2[vertices2.size()-2];
		
		float angle = getAngleBetweenPoints(p1, p2, p3);
		if(angle<returnAngle) {
			returnAngle =angle;
			startClosest1 = false;
			startClosest2 = false;
		}
		
	}
	if(glm::distance(start1,end2)<tolerance) {
		
		
		p1 = vertices1[1];
		p2 = vertices1[0];
		p3 = vertices2[vertices2.size()-2];
		
		float angle = getAngleBetweenPoints(p1, p2, p3);
		
		if(angle<returnAngle) {
			returnAngle =angle;
			startClosest1 = true;
			startClosest2 = false;
		}
		
	}
	
	if(returnAngle==360) {
		ofLog(OF_LOG_ERROR, "ofxLaser::Graphic::joinPolylines() - lines aren't touching");
		return false;
	} else {
		
		vector<glm::vec3>& poly1Vertices = poly1.getVertices();
		const vector<glm::vec3>& poly2Vertices = poly2.getVertices();
		
		if(!startClosest1 && startClosest2) {
			// connect new line to end of existing line
			//ofLog(OF_LOG_NOTICE, "connecting new start to existing end");
			poly1.addVertices(poly2Vertices);
		
			// new line start compared to existing line start
		} else if(startClosest1 && startClosest2) {
			// connect new line to start of existing line
			// reverse new poly

			//ofLog(OF_LOG_NOTICE, "connecting new start to existing start");
			for(int j=0; j<poly2Vertices.size(); j++) {
				poly1.insertVertex(poly2Vertices[j], 0);
			}

			// new line end connected to existing line end
		} else if(!startClosest1 && !startClosest2) {
			
			//ofLog(OF_LOG_NOTICE, "connecting new end to existing end");
			for(int j=(int)poly2Vertices.size()-1; j>=0; j--) {
				poly1.addVertex(poly2Vertices[j]);
			}



		// new line end compared to existing line start
		} else if(startClosest1 && !startClosest2) {
			//ofLog(OF_LOG_NOTICE, "connecting new end to existing start");
			for(int j=(int)poly2Vertices.size()-1; j>=0; j--) {
				//ofLog(OF_LOG_NOTICE, " - inserting element "+ofToString(j)+" into poly1 : " + ofToString(poly2Vertices[j]));
				poly1.insertVertex(poly2Vertices[j], 0);
			}

		
		}

		return true;
	}
	
}


// if polys are touching, this returns the angle
// between them, otherwise it returns 360

float Graphic :: comparePolylines(ofPolyline& poly1, ofPolyline &poly2) {
	
	const vector<glm::vec3>& vertices1 = poly1.getVertices();
	const vector<glm::vec3>& vertices2 = poly2.getVertices();
	
	const glm::vec3& start1 = vertices1.front();
	const glm::vec3& end1 = vertices1.back();
	const glm::vec3& start2 = vertices2.front();
	const glm::vec3& end2 = vertices2.back();
	
	float tolerance = 0.00001f;
	ofPoint p1, p2, p3;
	
	float returnAngle = 360;
//	glm::tvec3 testpoint;
//	testpoint.
	
	
	//if(end1.distance(start2)<tolerance) {
	if(glm::distance(end1, start2)<tolerance) {
			

		p1 = vertices1[vertices1.size()-2];
		p2 = vertices1.back();
		p3 = vertices2[1];
		
		float angle = getAngleBetweenPoints(p1, p2, p3);
		if(angle<returnAngle) returnAngle =angle;
	}
	if(glm::distance(start1,start2)<tolerance) {
		
		
		p1 = vertices1[1];
		p2 = vertices1[0];
		p3 = vertices2[1];
		
		float angle = getAngleBetweenPoints(p1, p2, p3);
		if(angle<returnAngle) returnAngle =angle;
	}
	if(glm::distance(end1,end2)<tolerance) {
		
		
		p1 = vertices1[vertices1.size()-2];
		p2 = vertices1.back();
		p3 = vertices2[vertices2.size()-2];
		
		float angle = getAngleBetweenPoints(p1, p2, p3);
		if(angle<returnAngle) returnAngle =angle;
	}
	
	if(glm::distance(start1, end2)<tolerance) {
		
		p1 = vertices1[1];
		p2 = vertices1[0];
		p3 = vertices2[vertices2.size()-2];
		
		float angle = getAngleBetweenPoints(p1, p2, p3);
		if(angle<returnAngle) returnAngle =angle;
	}
	return returnAngle;
	
}

float Graphic :: getAngleBetweenPoints(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3){
	
	glm::vec3 v1 = p2-p1;
	glm::vec3 v2 = p3-p2;
	v1 = glm::normalize(v1);
	v2 = glm::normalize(v2);
	
	return acos(glm::dot(v1,v2));

}

void Graphic :: clear() {
	for(ofPolyline* poly : polylines) Factory::releasePolyline(poly);
    polylines.clear();
    colours.clear(); 
	polylineMask.clear();
}


void Graphic :: serialize(ofJson&json) {
	ofJson& jsonPolylines = json["polylines"];
	for(ofPolyline* poly : polylines) {
		ofJson polyjson;
		serializePoly(polyjson, *poly);
		jsonPolylines.push_back(polyjson);
	}
	ofJson& jsonColours = json["colours"];
	for(ofColor& colour : colours) {
		jsonColours.push_back(colour.getHex());
	}
	
	ofJson& jsonMask = json["polymask"];
	for(ofPolyline& poly : polylineMask) {
		ofJson polyjson;
		serializePoly(polyjson, poly);
		jsonMask.push_back(polyjson);
	}
	
	//cout << json.dump() << endl;
	
}
void Graphic :: deserialize(ofJson&json) {
	
	clear();

	ofJson& jsonPolylines = json["polylines"];
	for (ofJson::iterator it = jsonPolylines.begin(); it != jsonPolylines.end(); ++it) {
		ofJson polylineData = *it;
		ofPolyline* newPoly = Factory::getPolyline(); // make a copy;
		deserializePoly(polylineData, *newPoly);
		polylines.push_back(newPoly);
	}
	ofJson& jsonColours = json["colours"];
	for (ofJson::iterator it = jsonColours.begin(); it != jsonColours.end(); ++it) {
		ofJson colourdata = *it;
		colours.push_back(ofColor::fromHex(colourdata));
	}
	ofJson& jsonMask = json["polymask"];
	for (ofJson::iterator it = jsonMask.begin(); it != jsonMask.end(); ++it) {
		ofJson polylineData = *it;
		ofPolyline newPoly;
		deserializePoly(polylineData, newPoly);
		polylineMask.push_back(newPoly);
	}

}

void Graphic::serializePoly(ofJson& json, ofPolyline& poly) {
	auto & vertices =  poly.getVertices();
	if(!vertices.empty()){
		ofJson& line = json["vertices"];
		for( size_t i=0; i<vertices.size(); i++ ){
			ofJson point;
			point["x"] = vertices[i].x;
			point["y"] = vertices[i].y;
			point["z"] = vertices[i].z;
			line.push_back(point);
		}
		
	}
}

void Graphic::deserializePoly(ofJson& json, ofPolyline& poly) {

	ofJson& linedata = json["vertices"];
	for (ofJson::iterator it = linedata.begin(); it != linedata.end(); ++it) {
		ofJson pointdata = *it;
		poly.addVertex(pointdata["x"], pointdata["y"], pointdata["z"]);
	}
		
	
}
