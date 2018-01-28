//
//  ofxLaserManager.cpp
//  ofxLaserRewrite
//
//  Created by Seb Lee-Delisle on 06/11/2017.
//
//

#include "ofxLaserManager.h"



using namespace ofxLaser;


Manager * Manager :: laserManager = NULL;

Manager * Manager::instance() {
	if(laserManager == NULL) {
		laserManager = new Manager();
	}
	return laserManager;
}


Manager :: Manager() {
	ofLog(OF_LOG_NOTICE, "ofxLaser::Manager constructor");
	if(laserManager == NULL) {
		laserManager = this;
	} else {
		ofLog(OF_LOG_WARNING, "Multiple ofxLaser::Manager instances created");
	}
	width = 800;
	height = 800;
	showPreview = true;
	showZones = false;
	currentProjector = -1;
	
	
	
	
}

void Manager :: setup(int w, int h){
	width = w;
	height = h;
}


void Manager::addProjector(DacBase& dac) {
	
	
	// create and add new projector object
	
	Projector* projector = new Projector("Projector"+ofToString(projectors.size()+1), dac);
	projectors.push_back(projector);
	// If we have no zones set up then create a big default zone.
	if(zones.size()==0) {
		addZoneToProjector(createDefaultZone(), projectors.size()-1);
		
	}
	
	Projector& proj = *projectors.back();

	//proj.gui->setPosition(width+320,10);
	
	//updateScreenSize();
	
}

void Manager::addZone(const ofRectangle& rect) {
	addZone(rect.x, rect.y, rect.width, rect.height);
	
}

void Manager :: addZone(float x, float y, float w, float h) {
	zones.push_back(new Zone("Zone"+ofToString(zones.size()+1),x, y, w, h));
	zones.back()->load();
	
}

void Manager::addZoneToProjector(int zonenum, int projnum) {
	if(projectors.size()<=projnum) {
		ofLog(OF_LOG_ERROR, "Invalid projector number passed to AddZoneToProjector(...)");
		return;
	}
	if(zones.size()<=zonenum) {
		ofLog(OF_LOG_ERROR, "Invalid zone number passed to AddZoneToProjector(...)");
		return;
	}
	
	projectors[projnum]->addZone(zones[zonenum], width, height);
	//projectorGuis[projnum]->add(projectors[projnum].params);
}

int Manager::createDefaultZone() {
	// check there aren't any zones yet?
	// create a zone equal to the width and height of the total output space
	addZone(0,0,width,height);
	return zones.size()-1;
	
}

void Manager::drawLine(const ofPoint& start, const ofPoint& end, const ofColor& col, string profileLabel) {
	
	// we should probably adjust the lines for the given glcontext at this point
	
	
	// store the shapes in the manager. Then we can figure out which zones
	// they belong in at draw time. In OFXLASER_ZONE_MANUAL we'll need to also store
	// which zone each shape belongs in.
	
	//Line l = new Line(gLProject(start), gLProject(end), ofFloatColor(col), 1, 1);
	Line* l = new Line(gLProject(start), gLProject(end), col, profileLabel);
	shapes.push_back(l);
	
	
}

void Manager::drawDot(const ofPoint& p, const ofColor& col, float intensity, string profileLabel) {
	
	// we should probably adjust the lines for the given glcontext at this point
	
	
	// store the shapes in the manager. Then we can figure out which zones
	// they belong in at draw time. In OFXLASER_ZONE_MANUAL we'll need to also store
	// which zone each shape belongs in.
	
	//Line l = new Line(gLProject(start), gLProject(end), ofFloatColor(col), 1, 1);
	Dot* d = new Dot(gLProject(p), col, intensity, profileLabel);
	shapes.push_back(d);
	
	
}

void Manager::drawPoly(const ofPolyline & poly, const ofColor& col, string profileName){
	
	// quick error check to make sure our line has any data!
	// (useful for dynamically generated lines, or empty lines
	// that are often found in poorly compiled SVG files)
	
	if((poly.getVertices().size()==0)||(poly.getPerimeter()<0.1)) return;
	
	//***
	// convert to 2D TODO -
	ofPolyline& polyline = tmpPoly;
	polyline.clear();

	for(int i = 0; i<poly.getVertices().size(); i++) {
		polyline.addVertex(gLProject(poly.getVertices()[i]));
	}
    
    polyline.setClosed(poly.isClosed());
    
	shapes.push_back(
					 new ofxLaser::Polyline(polyline, col, profileName) );
	
}

void Manager::drawPoly(const ofPolyline & poly, vector<ofColor>& colours, string profileName){
	
	// quick error check to make sure our line has any data!
	// (useful for dynamically generated lines, or empty lines
	// that are often found in poorly compiled SVG files)
	
	if((poly.getVertices().size()==0)||(poly.getPerimeter()<0.1)) return;
	

	ofPolyline& polyline = tmpPoly;
	polyline.clear();
	
	for(int i = 0; i<poly.getVertices().size(); i++) {
		polyline.addVertex(gLProject(poly.getVertices()[i]));
	}
	
	shapes.push_back(
					 new ofxLaser::Polyline(polyline, colours, profileName) );
	
}

void Manager::drawCircle(const ofPoint & centre, const float& radius, const ofColor& col,string profileName){
	
	
	shapes.push_back(
					 new ofxLaser::Circle(centre,radius, col, profileName) );
	
}

void Manager:: update(){
	
	// previously :
	// updated minpoints dependent on point speed
	// checked whether homography needed updating and updated where necessary
	// cleared all the points
	// cleared the preview mesh

	// delete all the shapes - all shape objects need a destructor!
	for(int i = 0; i<shapes.size(); i++) {
		delete shapes[i];
	}
	shapes.clear();
	bool updateZoneRects = false;
	for(int i = 0; i<zones.size(); i++) {
		zones[i]->visible= (currentProjector==-1);
		updateZoneRects = updateZoneRects | zones[i]->update();
	}
	for(int i = 0; i<projectors.size(); i++) {
		projectors[i]->update(updateZoneRects); // clears the points
	}
	
}
void Manager:: drawUI(bool fullscreen){
    
    ofPushStyle();
    ofNoFill();
    ofPushMatrix();
    if(currentProjector>=0) {
        ofTranslate(10,810);
        ofScale(0.3,0.3);
    } else {
        ofTranslate(0,0);
    }
    
    if(showPreview) {
        renderPreview();
    }
    // this renders the zones in the projector output space
    if(showZones) {


        for(int i = 0; i<zones.size(); i++) {
            zones[i]->draw();
        }
        
    }
    
    ofPopMatrix();
    ofPopStyle();
    
    
    if(currentProjector>-1) {
        for(int i = 0; i<projectors.size(); i++) {
            if(i==currentProjector)
                projectors[i]->drawUI(fullscreen);
            else
                projectors[i]->hideGui();

        }
        
    } else {
        for(int i = 0; i<projectors.size(); i++) {
            ofPushMatrix();
            ofTranslate(310*i+10,height+10);
            ofScale(3.0f/8.0f,3.0f/8.0f);
            ofSetDrawBitmapMode(OF_BITMAPMODE_SIMPLE) ;
            projectors[i]->drawUI();
            ofSetDrawBitmapMode(OF_BITMAPMODE_MODEL_BILLBOARD) ;
            ofPopMatrix();
        }
    }
 
    
    for(int i = 0; i<projectors.size(); i++) {
		projectors[i]->gui->setVisible(gui.isVisible());
		projectors[i]->gui->draw();
	}
    
    gui.draw();
    
	
}

void Manager :: renderPreview() {
    
    ofPushStyle();
    // draw outline of laser output area
    ofSetColor(255);
    ofNoFill();
    ofDrawRectangle(0,0,width,height);
    
    // Draw laser graphics preview ----------------
    ofMesh mesh;
    ofEnableBlendMode(OF_BLENDMODE_ADD);
    mesh.clear();
    mesh.setMode(OF_PRIMITIVE_LINE_STRIP);
    
    // Draw the preview laser graphics, with zones overlaid
    for(int i = 0; i<shapes.size(); i++) {
        shapes[i]->addPreviewToMesh(mesh);
    }
    
    ofSetLineWidth(1.5);
    mesh.draw();
    
    vector<ofFloatColor>& colours = mesh.getColors();
    
    for(int i = 0; i<colours.size(); i++) {
        colours[i].r*=0.4;
        colours[i].g*=0.4;
        colours[i].b*=0.4;
    }
    
    ofSetLineWidth(4);
    mesh.draw();
    
    ofDisableBlendMode();
    ofPopStyle();
    //----------------
    
}

void Manager :: updateScreenSize() {
	screenHeight = ofGetHeight();
	int spacing = 240;
	for(int i = 0; i<projectors.size(); i++) {
		int x = ofMap(i, 0, projectors.size(),ofGetWidth()-(spacing*projectors.size()), ofGetWidth());
		projectors[i]->gui->setPosition(x, 10);
	}
									
	gui.setPosition(ofGetWidth()-(spacing * (projectors.size()+1)),10);
	
}

void Manager::send(){
    
    // here's where the magic happens.
    // 1 :
    // figure out which zones to send the shapes to
    // and send them. When the zones get the shape, they transform them
    // into local zone space.
    for(int j = 0; j<zones.size(); j++) {
        Zone& z = *zones[j];
        z.shapes.clear();
        
        for(int i = 0; i<shapes.size(); i++) {
            Shape* s = shapes[i];
            // if (zone should have shape) then
            // TODO zone intersect shape test
            z.addShape(s);
        }
    }
    
    // 2 :
    // The projectors go through each of their zones, and pull out each shape
    // it'd need to be in zone space, then as each shape is converted to points, that's
    // when we'd do the warp for the projector space.
    
    // So - the shapes need to be sorted in projector space but their points need to be
    // calculated at zone space. Otherwise the perspective distortion won't look right in
    // terms of brightness distribution.
    for(int i = 0; i<projectors.size(); i++) {
        
        Projector& p = *projectors[i];
        
        p.send();
        
    }
    
    
    
}

int Manager :: getProjectorPointRate(int projectornum ){
    
    return projectors.at(projectornum)->getPointRate();
    
}

float Manager :: getProjectorFrameRate(int projectornum ){
    
    return projectors.at(projectornum)->getFrameRate();
    
}
void Manager::sendRawPoints(const vector<ofxLaser::Point>& points, int projectornum, int zonenum){
   // ofLog(OF_LOG_NOTICE, "ofxLaser::Manager::sendRawPoints(...) point count : "+ofToString(points.size())); 
    Projector* proj = projectors.at(projectornum);
    proj->sendRawPoints(points, zonenum);

}


void Manager::nextProjector() {
	currentProjector++;
	if(currentProjector>=projectors.size()) currentProjector=-1; 
	
}

void Manager::initGui() {
	
	
	ofxGuiSetDefaultWidth(220);
	ofxGuiSetFillColor(ofColor::fromHsb(144,100,112));
	gui.setup("Laser", "laserSettings.xml");
	gui.add(showZones.set("Show Zones", true));
	gui.add(showPreview.set("Show Preview", true));
	
//	
//	gui.add(laser.redParams);
//	gui.add(laser.greenParams);
//	gui.add(laser.blueParams);
//	
	gui.loadFromFile("laserSettings.xml");
    showPreview = true;
	gui.setPosition(width+10, 10);
	
	// TODO - check font exists?
	//gui.loadFont("fonts/Verdana.ttf", 8, false);
	
	
	for(int i = 0; i<projectors.size(); i++) {
		projectors[i]->initGui();
	}
	
	updateScreenSize();

}

void Manager::saveSettings() {
	gui.save();
	for(int i = 0; i<projectors.size(); i++) {
		projectors[i]->saveSettings();
		

	}
	
	
}

// converts openGL coords to screen coords //
ofPoint Manager::gLProject(ofPoint p) {
	return gLProject(p.x, p.y, p.z);
	
}
ofPoint Manager::gLProject( float ax, float ay, float az ) {
	
	//return ofPoint(ax, ay, 0);
	
	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);
	
	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);
	
	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	GLdouble X, Y, Z = 0;
	gluProject( ax, ay, az, model_view, projection, viewport, &X, &Y, &Z);
	
	// bit of a hack - if you're rendering into an Fbo then y is inverted
	if(projection[5]<0) Y = screenHeight-Y;//ofGetHeight()-Y;
   // return(ofPoint(ax, ay));
	return ofPoint(X, Y, 0.0f);
	
}

