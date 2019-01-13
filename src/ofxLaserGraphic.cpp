//
//  ofxLaser :: Graphic
//
//  Created by Seb Lee-Delisle on 01/03/2018.
//
//

#include "ofxLaserGraphic.h"
using namespace ofxLaser;
//
//const vector<int> Graphic::something = list_of(3)(5);

// static class members
vector<ofPolyline*> Graphic::polylinePool;
vector<ofPolyline*> Graphic::polylineSpares;
int Graphic::numGraphicsInMemory = 0;

void Graphic :: addSvg(ofxSVG& svg, bool optimise) {
	
	vector<ofPolyline*> newpolylines;
	vector<ofColor> colours;
	
	const vector <ofPath> & paths = svg.getPaths();
	
	for(int i = 0; i<paths.size(); i++ ) { // for (ofPath shape : svg.getPaths()){
		ofPath path = paths[i];
		path.setPolyWindingMode(OF_POLY_WINDING_ODD);
		if(path.isFilled()){
			
			subtractPathFromPolylines(path, newpolylines, colours);
			
		}
		if(path.hasOutline()){
			
			const vector<ofPolyline> & outlines = path.getOutline();
			ofColor col(path.getStrokeColor());
			for(const ofPolyline& line:outlines) {
				ofPolyline* newline = Graphic::getPolyline(&line);
			
				newpolylines.push_back(newline);
				colours.push_back(col);
			}
			
		}
		
	}
	for(int i = 0; i<newpolylines.size(); i++) {
		addPolyline(*newpolylines[i], colours[i], false);
	
	}

	for(ofPolyline* poly : newpolylines) {

		Graphic::releasePolyline(poly);

	}
	
	if(optimise) {
		for(int i = 0; i<polylines.size(); i++) {
			polylines[i]->simplify();
		}
		connectLineSegments();
	}
	
	
}

void Graphic::subtractPathFromPolylines(ofPath& sourcepath, vector<ofPolyline*>& targetpolys, vector<ofColor>& colours) {
	
	// Can we just call the polyline version on all shapes in the path?
	// - answer - no because ofPaths have compound shapes inside!
	
	
	vector<ofPolyline*> allNewPolys;
	vector<ofColor> allNewColours;
	
	for(vector<ofPolyline*>::iterator polyit = targetpolys.begin(); polyit != targetpolys.end();) {
		
		ofPolyline& targetpoly = **polyit; // polys[i];
		
		vector<ofPolyline*> polysToAdd;
		bool overlaps = false;
		
		// TODO fast out for overlap between path and shape
		
		// resample target shape - this clones ofPolyline - nothing we can do about that :/
		ofPolyline resampledPoly = targetpoly;//.getResampledBySpacing(1);
		vector<glm::vec3>& points = resampledPoly.getVertices();
		
		bool inside = false;
		for(vector<glm::vec3>::iterator it = points.begin(); it != points.end();)
		{
			
			if(pointInsidePath(*it, sourcepath)) {
				it = points.erase(it);
				overlaps = true;
				inside = false;
			} else {
				if(!inside) {
					polysToAdd.push_back(Graphic::getPolyline());
					inside = true;
				}
				polysToAdd.back()->addVertex(*it);
				++it;
			}
			
		}
		
		// if the shape overlaps then we have to deal with it,
		if(overlaps) {
			// firstly by deleting the colours and the shape completely...
			vector<ofColor>::iterator colouriterator = colours.begin() + (polyit-targetpolys.begin());
			ofPolyline* polyToErase = *polyit;
			polyit = targetpolys.erase(polyit);
			// this assumes that the polyline was made by this app...
			// which isn't a good assumption the way that this function is written.
			Graphic::releasePolyline(polyToErase);
			
			ofColor newPolyColour = *colouriterator;
			colouriterator = colours.erase(colouriterator);
			//
			for(ofPolyline* polyToAdd : polysToAdd ) {
				allNewPolys.push_back(polyToAdd);
				allNewColours.push_back(newPolyColour);
			}
			//
			
		} else {
			// We should have one copy of the shape in the new polys list, let's delete it!
			if(polysToAdd.size()!=1) {
				ofLog(OF_LOG_WARNING, "Graphic::subtractPathFromPolylines");
			}
			for(ofPolyline* poly:polysToAdd) Graphic::releasePolyline(poly);
			polyit ++;
		}
		
		
	}
	
	if(allNewPolys.size()>0) {
		targetpolys.insert( targetpolys.end(), allNewPolys.begin(), allNewPolys.end() );
		colours.insert(colours.end(), allNewColours.begin(), allNewColours.end());
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


bool Graphic:: pointInsidePolylines(glm::vec3 point, const vector<ofPolyline*>& polys) {
	
	for(ofPolyline* poly : polys) {
		if(poly->inside(point)) {
			return true;
		}
		
	}
	return false;
	
}


void Graphic::subtractShapeFromPolylines(const ofPolyline& polyToSubtract, vector<ofPolyline*>& targetpolys, vector<ofColor>& colours) {
	
	//ofPolyline& polyToSubtract = sourceshape;

	// do we need this?
//	if(!polyToSubtract.isClosed()) {
//		polyToSubtract.close();
//	}
	vector<ofPolyline*> allNewPolys;
	vector<ofColor> allNewColours;
	
	for(vector<ofPolyline*>::iterator polyit = targetpolys.begin(); polyit != targetpolys.end();) {
		
		ofPolyline& targetpoly = **polyit; // polys[i];
		
		vector<ofPolyline*> polysToAdd;
		bool overlaps = false;
		
		if(polyToSubtract.getBoundingBox().intersects(targetpoly.getBoundingBox())) {
			
			// resample target shape
			// TODO make this work with lines, not points...
			ofPolyline resampledPoly = targetpoly.getResampledBySpacing(1);
			vector<glm::vec3>& points = resampledPoly.getVertices();
			
			bool inside = false;
			for(vector<glm::vec3>::iterator it = points.begin(); it != points.end();)
			{
				
				if(polyToSubtract.inside(*it))
				{
					it = points.erase(it);
					overlaps = true;
					inside = false;
				}
				else
				{
					if(!inside) {
						polysToAdd.push_back(Graphic::getPolyline());
						inside = true;
					}
					polysToAdd.back()->addVertex(*it);
					++it;
				}
				
			}
			
		}
		
		if(overlaps) {
			vector<ofColor>::iterator colouriterator = colours.begin() + (polyit-targetpolys.begin());
			
			
			ofPolyline* polyToErase = *polyit;
			polyit = targetpolys.erase(polyit);
			Graphic::releasePolyline(polyToErase);
			
			
			
			ofColor newPolyColour = *colouriterator;
			colouriterator = colours.erase(colouriterator);
			
			for(ofPolyline* polyToAdd : polysToAdd ) {
				allNewPolys.push_back(polyToAdd);
				allNewColours.push_back(newPolyColour);
			}

		} else {
			polyit ++;
		}
		
	}
	
	if(allNewPolys.size()>0) {
		targetpolys.insert( targetpolys.end(), allNewPolys.begin(), allNewPolys.end() );
		colours.insert(colours.end(), allNewColours.begin(), allNewColours.end());
	}
	
}


void Graphic::subtractShapesFromPolylines(const vector<ofPolyline*>& polysToSubtract, vector<ofPolyline*>& targetpolys, vector<ofColor>& colours) {
	
	//ofPolyline& polyToSubtract = sourceshape;
	
	// do we need this?
	//	if(!polyToSubtract.isClosed()) {
	//		polyToSubtract.close();
	//	}
	vector<ofPolyline*> allNewPolys;
	vector<ofColor> allNewColours;
	
	for(vector<ofPolyline*>::iterator polyit = targetpolys.begin(); polyit != targetpolys.end();) {
		
		ofPolyline& targetpoly = **polyit; // polys[i];
		
		vector<ofPolyline*> polysToAdd;
		bool overlaps = false;
		
	
		// resample target shape
		ofPolyline resampledPoly = targetpoly.getResampledBySpacing(1);
		vector<glm::vec3>& points = resampledPoly.getVertices();
		
		bool inside = false;
		for(vector<glm::vec3>::iterator it = points.begin(); it != points.end();)
		{
			
			if( pointInsidePolylines( *it, polysToSubtract)) {
			
				it = points.erase(it);
				overlaps = true;
				inside = false;
			} else {
				if(!inside) {
					polysToAdd.push_back(Graphic::getPolyline());
					inside = true;
				}
				polysToAdd.back()->addVertex(*it);
				++it;
			}
			
		}
		
	
		
		if(overlaps) {
			vector<ofColor>::iterator colouriterator = colours.begin() + (polyit-targetpolys.begin());
			ofPolyline* polyToErase = *polyit;
			polyit = targetpolys.erase(polyit);
			// this assumes that the polyline was made by this app...
			// which isn't a good assumption the way that this function is written.
			Graphic::releasePolyline(polyToErase);
			
			ofColor newPolyColour = *colouriterator;
			colouriterator = colours.erase(colouriterator);
			
			for(ofPolyline* polyToAdd : polysToAdd ) {
				allNewPolys.push_back(polyToAdd);
				allNewColours.push_back(newPolyColour);
			}
			
		} else {
			polyit ++;
		}
		
	}
	
	if(allNewPolys.size()>0) {
		targetpolys.insert( targetpolys.end(), allNewPolys.begin(), allNewPolys.end() );
		colours.insert(colours.end(), allNewColours.begin(), allNewColours.end());
	}
	
}

void Graphic :: translate(glm::vec3 offset) {
	
	for(int i = 0; i<polylines.size(); i++) {
		
		ofPolyline& poly =*polylines[i];
		poly.translate(offset);
		
	}
	
}

void Graphic :: autoCentre() {
	
	ofRectangle boundingBox;
	
	for(int i = 0; i<polylines.size(); i++) {
		
		ofPolyline& poly =*polylines[i];
		
		if(i==0) {
			boundingBox = poly.getBoundingBox();
		} else {
			boundingBox.growToInclude(poly.getBoundingBox());
		}
		
	}
	
	translate(-boundingBox.getCenter());
}

void Graphic :: addPolyline(const ofPolyline& poly, ofColor& colour, bool useTransform) {
	
	// we don't need no one point vertices!
	if(poly.getVertices().size()<2) {
		//ofLog(OF_LOG_NOTICE, "deleting single vertex line");
		return;
	}
	if(poly.getPerimeter()==0) {
		//ofLog(OF_LOG_NOTICE, "deleting empty line");
		return;
	}
	
	ofPolyline* newPoly = Graphic::getPolyline(&poly); // make a copy;

	if(useTransform) {
		transformPolyline(*newPoly);
	}
	polylines.push_back(newPoly);
	colours.push_back(colour);
	
	
}

void Graphic :: transformPolyline(ofPolyline& poly) {
	vector<glm::vec3>& newPolyVerts = poly.getVertices();

	for(int i = 0; i<newPolyVerts.size(); i++) {
		newPolyVerts[i]=gLProject(newPolyVerts[i]);
	}
	
}

void Graphic :: addPolylineUnder(const ofPolyline& poly, ofColor& colour, bool useTransform) {
	
	ofPolyline* newpoly = Graphic::getPolyline(&poly);
	// make a vector of polylines containing this new poly
	if(useTransform) {
		transformPolyline(*newpoly);
	}
	vector<ofPolyline*> newpolys;
	vector<ofColor>newcolours;
	newpolys.push_back(newpoly);
	newcolours.push_back(colour);
	
	// go through every polyline

	// subtract it from our original, and store the pieces in the vector
	subtractShapesFromPolylines(polylines, newpolys, newcolours);
		

	// now add the pieces to our graphic!
	for(int i = 0; i<newpolys.size(); i++) {
		addPolyline(*newpolys[i], newcolours[i], false);
		
	}
	for(ofPolyline* poly : newpolys) {
		Graphic::releasePolyline(poly);
	}
	
	
}

glm::vec3 Graphic::gLProject(glm::vec3& v) {
	
	float ax = v.x;
	float ay = v.y;
	float az = v.z;
	
	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	
	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	
	GLdouble X, Y, Z = 0;
	gluProject( ax, ay, az, model_view, projection, viewport, &X, &Y, &Z);
	
	// bit of a hack - if you're rendering into an Fbo then y is inverted
	if(projection[5]<0) Y = ofGetHeight()-Y;
	// return(ofPoint(ax, ay));
	return glm::vec3(X, Y, 0.0f);
	
}

void Graphic ::  connectLineSegments() {
	
	for(int i = 0; i<polylines.size(); i++) {
		ofPolyline* poly1 = polylines[i];
		
		float smallestAngle = 360;
		int closestIndex = -1;
		
		for(int j = i+1; j<polylines.size(); j++) {
			
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
				Graphic::releasePolyline(polyToErase);
				
				// so we test the newly joined poly again
				i--;
			}
		}
		
		
	}
	
	
}
bool Graphic :: joinPolylines(ofPolyline& poly1, ofPolyline &poly2) {
	
	auto& vertices1 = poly1.getVertices();
	auto& vertices2 = poly2.getVertices();
	
	auto& start1 = vertices1.front();
	auto& end1 = vertices1.back();
	auto& start2 = vertices2.front();
	auto& end2 = vertices2.back();
	
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
		
		auto& poly1Vertices = poly1.getVertices();
		auto& poly2Vertices = poly2.getVertices();
		
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
			for(int j=poly2Vertices.size()-1; j<=0; j--) {
				poly1.addVertex(poly2Vertices[j]);
			}

			// new line end compared to existing line start
		} else if(startClosest1 && !startClosest2) {
			//ofLog(OF_LOG_NOTICE, "connecting new end to existing start");
			for(int j=poly2Vertices.size()-1; j>=0; j--) {
				//ofLog(OF_LOG_NOTICE, " - inserting element "+ofToString(j)+" into poly1 : " + ofToString(poly2Vertices[j]));
				poly1.insertVertex(poly2Vertices[j], 0);
			}
		
		}

		return true;
	}
	
}



//
//	glm::vec3 newPolyStart = newPolyVerts.front();
//	glm::vec3 newPolyEnd = newPolyVerts.back();
//
//	//		int nearestPoly = -1;
//	//		bool nearestToStart = false;
//	//		bool newReversed = false;
//	//		float angle = INFINITY;
//	bool connected = false;
//
//	float tolerance = 1.0f;
//
//	for(int i = 0; i<polylines.size();i++) {
//
//		if(colour!=colours[i]) continue;
//
//		ofPolyline& existingPoly = polylines[i];
//		glm::vec3 existingPolyStart = existingPoly.getVertices().front();
//		glm::vec3 existingPolyEnd = existingPoly.getVertices().back();
//
//		//ofLog(OF_LOG_NOTICE, ofToString(existingPolyStart)+ " " + ofToString(existingPolyEnd)+ " "+ ofToString(newPolyStart) + " " + ofToString(newPolyEnd));
//
//		// new line start compared to existing line end
//		if(newPolyStart.distance(existingPolyEnd) < tolerance) {
//			// connect new line to end of existing line
//			//ofLog(OF_LOG_NOTICE, "connecting new start to existing end");
//			existingPoly.addVertices(newPolyVerts);
//			connected = true;
//			break;
//
//			// new line start compared to existing line start
//		} else if(newPolyStart.distance(existingPolyStart) < tolerance) {
//			// connect new line to start of existing line
//			// reverse new poly
//
//			//ofLog(OF_LOG_NOTICE, "connecting new start to existing start");
//			for(int j=0; j<newPolyVerts.size(); j++) {
//				existingPoly.insertVertex(newPolyVerts[j], 0);
//			}
//			connected = true;
//			break;
//
//			// new line end compared to existing line end
//		} else if(newPolyEnd.distance(existingPolyEnd) < tolerance) {
//			//ofLog(OF_LOG_NOTICE, "connecting new end to existing end");
//			for(int j=newPolyVerts.size()-1; j<=0; j--) {
//				existingPoly.addVertex(newPolyVerts[j]);
//			}
//			connected = true;
//			break;
//
//			// new line end compared to existing line start
//		} else if(newPolyEnd.distance(existingPolyStart) < tolerance) {
//			//ofLog(OF_LOG_NOTICE, "connecting new end to existing start");
//			for(int j=newPolyVerts.size()-1; j<=0; j--) {
//				existingPoly.insertVertex(newPolyVerts[j], 0);
//			}
//			connected = true;
//			break;
//		}
//
//	}
//

// if polys are touching, this returns the angle
// between them, otherwise it returns 360

float Graphic :: comparePolylines(ofPolyline& poly1, ofPolyline &poly2) {
	
	auto& vertices1 = poly1.getVertices();
	auto& vertices2 = poly2.getVertices();
	
	auto& start1 = vertices1.front();
	auto& end1 = vertices1.back();
	auto& start2 = vertices2.front();
	auto& end2 = vertices2.back();
	
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

void Graphic :: renderToLaser(ofxLaser::Manager& laser, float brightness, string renderProfile) {
	
	//laser.setTargetZone(targetZone); // only relevant if we're in OFXLASER_ZONE_MANUAL
	
	for(int i = 0; i<polylines.size(); i++) {
		ofColor col = colours[i];
		col*=brightness;
		laser.drawPoly(*polylines[i],col, renderProfile);
		
	}
	
}
void Graphic :: clear() {
    polylines.clear();
    colours.clear(); 
    
}


