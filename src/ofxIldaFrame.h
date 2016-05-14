//
//  ofxIldaFrame.h
//  ofxIlda
//
//  Created by Memo Akten on 09/05/2013.
//  Updated by Mitsuru Takeuchi on 02/06/2013.
//


// a single ILDA frame, contains multiple polys
// coordinates are NORMALIZED (0...1, 0...1)

#pragma once

#include "ofMain.h"
#include "ofxIldaPoly.h"
#include "ofxIldaPoint.h"
#include "ofxIldaPolyProcessor.h"


namespace ofxIlda {
	
    class Frame {
    public:
        struct {
            struct {
                bool lines; // draw lines
                bool points;    // draw points
                bool pointNumbers;  // draw point numbers (not implemented yet)
            } draw;
            
            struct {
                ofFloatColor color; // color
                int blankCount;     // how many blank points to send at path ends
                int endCount;       // how many end repeats to send
                bool doCapX;        // cap out of range on x (otherwise wraps around)
                bool doCapY;        // cap out of range on y (otherwise wraps around)
                struct {
                    bool doFlipX;
                    bool doFlipY;
                    ofVec2f offset;
                    ofVec2f scale;
                } transform;
            } output;
        } params;
        
        PolyProcessor polyProcessor; // params and functionality for processing the polys
        
        struct {
            int pointCountOrig;    // READONLY current total number of points across all paths (excluding blanks and end repititons)
            int pointCountProcessed; // same as above, except AFTER being processed
        } stats;
        
        
        //--------------------------------------------------------------
        Frame() {
            setDefaultParams();
        }
        
        //--------------------------------------------------------------
        void setDefaultParams() {
            memset(&params, 0, sizeof(params));  // safety catch all default to zero
            memset(&stats, 0, sizeof(stats));  // safety catch all default to zero
            
            params.draw.lines = true;
            params.draw.points = true;
            params.draw.pointNumbers = false;
            
            params.output.color.set(1, 1, 1, 1);
            params.output.blankCount = 30;
            params.output.endCount = 30;
            params.output.doCapX = false;
            params.output.doCapY = false;
            
            params.output.transform.doFlipX = false;
            params.output.transform.doFlipY = false;
            params.output.transform.offset.set(0, 0);
            params.output.transform.scale.set(1, 1);
        }
        
        
        //--------------------------------------------------------------
        string getString() {
            stringstream s;
            s << polyProcessor.getString();
            
            s << "params:" << endl;
            s << "draw.lines : " << params.draw.lines << endl;
            s << "draw.point : " << params.draw.points << endl;
            s << "draw.pointNumbers : " << params.draw.pointNumbers << endl;
            
            s << "output.color : " << params.output.color << endl;
            s << "output.blankCount : " << params.output.blankCount << endl;
            s << "output.endCount : " << params.output.endCount << endl;
            s << "output.doCapX : " << params.output.doCapX << endl;
            s << "output.doCapY : " << params.output.doCapY << endl;
            s << "output.transform.doFlipX : " << params.output.transform.doFlipX << endl;
            s << "output.transform.doFlipY : " << params.output.transform.doFlipY << endl;
            s << "output.transform.offset : " << params.output.transform.offset << endl;
            s << "output.transform.scale : " << params.output.transform.scale << endl;
            
            s << endl;
            
            s << "stats:" << endl;
            s << "stats.pointCountOrig : " << stats.pointCountOrig << endl;
            s << "stats.pointCountProcessed : " << stats.pointCountProcessed << endl;
            
            return s.str();
        }
        
        //--------------------------------------------------------------
        void update() {
            polyProcessor.update(origPolys, processedPolys);
			
            // get stats
            stats.pointCountOrig = 0;
            stats.pointCountProcessed = 0;
            for(int i=0; i<processedPolys.size(); i++) {
                stats.pointCountOrig += origPolys[i].size();
                stats.pointCountProcessed += processedPolys[i].size();
            }
            
            updateFinalPoints();
        }
        
        
        //--------------------------------------------------------------
        void draw(float x=0, float y=0, float w=0, float h=0) {
            if(w==0) w = ofGetWidth();
            if(h==0) h = ofGetHeight();
            
            ofPushStyle();
            ofPushMatrix();
            ofTranslate(x, y);
            ofScale(w, h);
            
            if(params.draw.lines) {
                ofSetLineWidth(2);
                for(int i=0; i<processedPolys.size(); i++) {
                    ofPolyline &poly = processedPolys[i];
					ofFloatColor &pcolor = processedPolys[i].color;
					ofSetColor(pcolor.r*255, pcolor.g*255, pcolor.b*255);
                    poly.draw();
                    //            for(int i=0; i<data.size(); i++) {
                    //                ofPoint p0 = data[i];
                    //                if(i < data.size()-1) {
                    //                    ofPoint p1 = data[i+1];
                    ////                    ofSetColor(p1.r * 255, p1.g * 255, p1.b * 255, p1.a * 255);
                    //                    ofDrawLine(p0.x, p0.y, p1.x, p1.y);
                    //                }
                    //            }
                }
            }
            if(params.draw.points) {
                glPointSize(5);
                for(int i=0; i<processedPolys.size(); i++) {
                    ofPolyline &poly = processedPolys[i];
                    ofFloatColor &pcolor = processedPolys[i].color;
					ofSetColor(pcolor.r*255, pcolor.g*255, pcolor.b*255);
                    
					glBegin(GL_POINTS);
                    for(int i=0; i<poly.size(); i++) {
                        ofPoint &p = poly[i];
                        //                Point &p = data[i];
                        //                ofSetColor(p.r * 255, p.g * 255, p.b * 255, p.a * 255);
                        glVertex2f(p.x, p.y);
                    }
                    glEnd();
                }
            }
            
            ofPopMatrix();
            ofPopStyle();
        }
        
        //--------------------------------------------------------------
        void clear() {
            origPolys.clear();
            processedPolys.clear();
        }
        
        //--------------------------------------------------------------
        int size() {
            return origPolys.size();
        }
        
        //--------------------------------------------------------------
        Poly& addPoly(const Poly& poly) {
            origPolys.push_back(poly);
            return origPolys.back();
        }
        
        //--------------------------------------------------------------
        Poly& addPoly() {
            return addPoly(Poly(params.output.color));
        }
        
        //--------------------------------------------------------------
        Poly& addPoly(const ofPolyline& polyline) {
            return addPoly(polyline, params.output.color);
        }
        
        //--------------------------------------------------------------
        Poly& addPoly(const ofPolyline& polyline, ofFloatColor color) {
            return addPoly(Poly(polyline, color));
        }

        //--------------------------------------------------------------
        Poly& addPoly(const vector<ofPoint> points) {
            return addPoly(Poly(points));
        }
        
        //--------------------------------------------------------------
        Poly& addPoly(const vector<ofPoint> points, ofFloatColor color) {
            return addPoly(Poly(points, color));
        }
        
        //--------------------------------------------------------------
        void addPolys(const vector<ofPolyline> &polylines) {
            for(int i=0; i<polylines.size(); i++) addPoly(polylines[i]);
        }
        
        //--------------------------------------------------------------
        void addPolys(const vector<ofPolyline> &polylines, ofFloatColor color) {
            for(int i=0; i<polylines.size(); i++) addPoly(polylines[i], color);
        }

        
        //--------------------------------------------------------------
        void addPolys(const vector<Poly> &polys) {
            for(int i=0; i<polys.size(); i++) addPoly(polys[i]);
        }
        
        //--------------------------------------------------------------
        Poly& getPoly(int i) {
            return origPolys[i];
        }
        
        //--------------------------------------------------------------
        Poly& getPolyProcessed(int i) {
            return processedPolys[i];
        }
        
        //--------------------------------------------------------------
        vector<Poly> &getPolys() {
            return origPolys;
        }
        
        //--------------------------------------------------------------
        vector<Poly> &getProcessedPolys() {
            return processedPolys;
        }
        
        //--------------------------------------------------------------
        Poly& getLastPoly() {
            if(origPolys.empty()) addPoly();
            return origPolys.back();
        }
        
        
        //--------------------------------------------------------------
        const vector<Point>& getPoints() const {
            return points;
        }
        
        //--------------------------------------------------------------
        void drawCalibration() {
            addPoly(Poly::fromRectangle(ofRectangle(0, 0, 1, 1)));
            ofPolyline &p1 = addPoly();
            p1.lineTo(0.25, 0.25);
            p1.lineTo(0.75, 0.75);
            
            ofPolyline &p2 = addPoly();
            p2.lineTo(0.75, 0.25);
            p2.lineTo(0.25, 0.75);
            
            ofPolyline &p3 = addPoly();
            float r = .25 * sqrt(2.0f);
            p3.arc(0.5, 0.5, r, r, 0, 360, 60);

        }

        //--------------------------------------------------------------
        ofPoint transformPoint(ofPoint p) const {
            // flip
            if(params.output.transform.doFlipX) p.x = 1 - p.x;
            if(params.output.transform.doFlipY) p.y = 1 - p.y;
            
            // scale
            if(params.output.transform.scale.lengthSquared() > 0) {
                p -= ofPoint(0.5, 0.5);
                p *= params.output.transform.scale;
                p += ofPoint(0.5, 0.5);
            }
            
            // offset
            p += params.output.transform.offset;
            


            
            // cap or wrap
            if(p.x < 0) {
                p.x = params.output.doCapX ? 0 : 1 + p.x - ceil(p.x);
            } else if(p.x > 1) {
                p.x = params.output.doCapX ? 1 : p.x - floor(p.x);
            }
            
            if(p.y < 0) {
                p.y = params.output.doCapY ? 0 : 1 + p.y - ceil(p.y);
            } else if(p.y > 1) {
                p.y = params.output.doCapY ? 1 : p.y - floor(p.y);
            }
            
            return p;
        }
        
        //--------------------------------------------------------------
        void updateFinalPoints() {
            points.clear();
            for(int i=0; i<processedPolys.size(); i++) {
                ofPolyline &poly = processedPolys[i];
                ofFloatColor &pcolor = processedPolys[i].color;
                
                if(poly.size() > 0) {
                    
                    ofPoint startPoint = transformPoint(poly.getVertices().front());
                    ofPoint endPoint = transformPoint(poly.getVertices().back());
                    
                    // blanking at start
                    for(int n=0; n<params.output.blankCount; n++) {
                        points.push_back( Point(startPoint, ofFloatColor(0, 0, 0, 0)));
                    }
                    
                    // repeat at start
                    for(int n=0; n<params.output.endCount; n++) {
                        points.push_back( Point(startPoint, pcolor) );
                    }
                    
                    // add points
                    for(int j=0; j<poly.size(); j++) {
                        points.push_back( Point(transformPoint(poly[j]), pcolor) );
                    }
                    
                    // repeat at end
                    for(int n=0; n<params.output.endCount; n++) {
                        points.push_back( Point(endPoint, pcolor) );
                    }
                    
                    // blanking at end
                    for(int n=0; n<params.output.blankCount; n++) {
                        points.push_back( Point(endPoint, ofFloatColor(0, 0, 0, 0) ));
                    }
                    
                }
            }
        }
        
    protected:
        vector<Poly> origPolys;   // stores the original polys
        vector<Poly> processedPolys;  // stores the processed (smoothed, collapsed, optimized, resampled etc).
        vector<Point> points;   // final points to send
    };
}