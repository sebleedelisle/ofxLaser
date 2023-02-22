//
//  ofxLaserTestPatternGenerator.h
//
//  Created by Seb Lee-Delisle on 09/02/2023.
//
//

#pragma once

#include "ofMain.h"
#include "ofxLaserCircle.h"
#include "ofxLaserLine.h"
#include "ofxLaserDot.h"
#include "ofxLaserManualShape.h"
#include "ofxLaserConstants.h"

namespace ofxLaser {
class TestPatternGenerator {
    public :
    
    static int getNumTestPatterns() {
        return 10;
    } 
    
    static vector<ofxLaser::Shape*> getTestPatternShapes(int testPattern, const ofRectangle& rect) {
        vector<Shape*> shapes;

         if(testPattern==1) {

             ofColor col = ofColor(0,255,0);
             shapes.push_back(new Line(rect.getTopLeft(), rect.getTopRight(), col, OFXLASER_PROFILE_FAST));
             shapes.push_back(new Line(rect.getTopRight(), rect.getBottomRight(), col, OFXLASER_PROFILE_FAST));
             shapes.push_back(new Line(rect.getBottomRight(), rect.getBottomLeft(), col, OFXLASER_PROFILE_FAST));
             shapes.push_back(new Line(rect.getBottomLeft(), rect.getTopLeft(), col, OFXLASER_PROFILE_FAST));
             shapes.push_back(new Line(rect.getTopLeft(), rect.getBottomRight(), col, OFXLASER_PROFILE_FAST));
             shapes.push_back(new Line(rect.getTopRight(), rect.getBottomLeft(), col, OFXLASER_PROFILE_FAST));
             
             float cornersize = rect.getWidth()*0.2;
             float spacer = rect.getWidth()*0.05;
             
             if(cornersize > rect.getHeight()*0.5) {
                 cornersize = rect.getHeight()*0.5;
                 spacer = cornersize*0.25;
             }
             
             shapes.push_back(new Line(rect.getTopLeft()+glm::vec3(spacer,spacer, 0), rect.getTopLeft()+glm::vec3(cornersize,spacer,0), ofColor::white, OFXLASER_PROFILE_FAST));
             shapes.push_back(new Line(rect.getTopLeft()+glm::vec3(spacer,spacer, 0), rect.getTopLeft()+glm::vec3(spacer,cornersize,0), ofColor::white, OFXLASER_PROFILE_FAST));
           //shapes.push_back(new Circle(rect.getTopLeft()+glm::vec3(cornersize,cornersize, 0), cornersize*0.8, ofColor::white, OFXLASER_PROFILE_FAST));


         } else if(testPattern==2) {

           
             ofPoint v = rect.getBottomRight() - rect.getTopLeft()-ofPoint(0.2,0.2);
             for(float y = 0; y<=1.1; y+=0.333333333) {
                 shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1, rect.getTop()+0.1+v.y*y),ofPoint(rect.getRight()-0.1, rect.getTop()+0.1+v.y*y), ofColor(255), OFXLASER_PROFILE_FAST));
             }

             for(float x =0 ; x<=1.1; x+=0.3333333333) {
                 shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1+ v.x*x, rect.getTop()+0.1),ofPoint(rect.getLeft()+0.1 + v.x*x, rect.getBottom()-0.1), ofColor(255,0,0), OFXLASER_PROFILE_FAST ));
             }

             shapes.push_back(new Circle(rect.getCenter(), rect.getWidth()/12, ofColor(0,0,255), OFXLASER_PROFILE_DEFAULT));
             shapes.push_back(new Circle(rect.getCenter(), rect.getWidth()/6, ofFloatColor(0,1,0), OFXLASER_PROFILE_DEFAULT));


         }else if(testPattern==3) {

         
             ofPoint v = rect.getBottomRight() - rect.getTopLeft()-ofPoint(0.2,0.2);

             for(float y = 0; y<=1.1; y+=0.333333333) {

                 shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1, rect.getTop()+0.1+v.y*y),ofPoint(rect.getRight()-0.1, rect.getTop()+0.1+v.y*y), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));
             }
             shapes.push_back(new Line(rect.getTopLeft(),  glm::mix( rect.getTopLeft(), rect.getBottomLeft(), 1.0f/3.0f ), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));

             shapes.push_back(new Line(rect.getBottomLeft(), glm::mix(rect.getTopLeft(), rect.getBottomLeft(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));

             shapes.push_back(new Line( glm::mix(rect.getTopRight(), rect.getBottomRight(), 1.0f/3.0f), mix(rect.getTopRight(), rect.getBottomRight(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));


         } else if(testPattern==4) {

          
             ofPoint v = rect.getBottomRight() - rect.getTopLeft()-ofPoint(0.2,0.2);

             for(float x =0 ; x<=1.1; x+=0.3333333333) {
                 shapes.push_back(new Line(ofPoint(rect.getLeft()+0.1+ v.x*x, rect.getTop()+0.1),ofPoint(rect.getLeft()+0.1 + v.x*x, rect.getBottom()-0.1), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT ));

             }

             shapes.push_back(new Line(rect.getTopLeft(), glm::mix( rect.getTopLeft(), rect.getTopRight(), 1.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));

             shapes.push_back(new Line(rect.getTopRight(), glm::mix( rect.getTopLeft(), rect.getTopRight(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));

             shapes.push_back(new Line(glm::mix(rect.getBottomLeft(), rect.getBottomRight(), 1.0f/3.0f), glm::mix(rect.getBottomLeft(), rect.getBottomRight(), 2.0f/3.0f), ofColor(0,255,0), OFXLASER_PROFILE_DEFAULT));


         } else if((testPattern>=5) && (testPattern<=8)) {
             ofColor c;

             ofRectangle thinrect = rect;
             thinrect.scaleFromCenter(0.5, 0.1);
             
             vector<ofPoint> points;
             vector<ofColor> colours;

             ofPoint currentPosition = thinrect.getTopLeft();

             for(int row = 0; row<5; row ++ ) {


                 float y =thinrect.getTop() + (thinrect.getHeight()*row/4);

                 ofPoint left = ofPoint(thinrect.getLeft(), y);

                 ofPoint right = ofPoint(thinrect.getRight(), y);

                 int moveIterations = currentPosition.distance(left)/1;

                 for(int i = 0; i<moveIterations; i++) {
                     points.push_back(currentPosition.getInterpolated(left, (float)i/(float)moveIterations));
                     colours.push_back(ofColor(0));

                 }
                 currentPosition = right;

                 if(testPattern == 5) c.set(255,0,0);
                 else if(testPattern == 6) c.set(0,255,0);
                 else if(testPattern == 7) c.set(0,0,255);
                 else if(testPattern == 8) c.set(255,255,255);

                 switch (row) {
                     case 0 :
//                         c.r *= colourSettings.red100;
//                         c.g *= colourSettings.green100;
//                         c.b *= colourSettings.blue100;
                         break;
                     case 1 :
                         c.r *= 0.75;//colourSettings.red75;
                         c.g *= 0.75;//colourSettings.green75;
                         c.b *= 0.75;//colourSettings.blue75;
                         break;
                     case 2 :
                         c.r *= 0.5;//colourSettings.red50;
                         c.g *= 0.5;//colourSettings.green50;
                         c.b *= 0.5;//colourSettings.blue50;
                         break;
                     case 3 :
                         c.r *= 0.25;//colourSettings.red25;
                         c.g *= 0.25;//colourSettings.green25;
                         c.b *= 0.25;//colourSettings.blue25;
                         break;
                     case 4 :
                         c.r *= 0.0;//colourSettings.red0;
                         c.g *= 0.0;//colourSettings.green0;
                         c.b *= 0.0;//colourSettings.blue0;
                         break;
                 }

                 float speed = 10 * ( 1- (row*0.25));
                 if(speed<2.5) speed = 2.5;

                 int blanks = 5;
                 for(int i = 0; i< blanks; i++) {
                     points.push_back(left);
                     colours.push_back(ofColor(0));
                 }
                 for(float x =left.x ; x<=right.x; x+=speed) {
                     points.push_back(ofPoint(x,y));
                     colours.push_back(c);
                 }

                 for(int i = 0; i< blanks; i++) {
                     points.push_back(right);
                     colours.push_back(ofColor(0));
                 }


             }
             // WAIT... why am i manually doing the colour calibration, surely
             // I can just set it to 0, 0.25, 0.5 etc and let the colour calibration
             // fix it???
             
             shapes.push_back(new ManualShape(points, colours, true, OFXLASER_PROFILE_DEFAULT));

         } else if(testPattern ==9) {
          
             shapes.push_back(new Dot(rect.getTopLeft(), ofColor(255,255,255), 1, OFXLASER_PROFILE_DEFAULT));
             shapes.push_back(new Dot(rect.getTopRight(), ofColor(255,255,255), 1, OFXLASER_PROFILE_DEFAULT));
             shapes.push_back(new Dot(rect.getBottomLeft(), ofColor(255,255,255), 1, OFXLASER_PROFILE_DEFAULT));
             shapes.push_back(new Dot(rect.getBottomRight(), ofColor(255,255,255), 1, OFXLASER_PROFILE_DEFAULT));

         } else if(testPattern ==10) {
       
             float progress = (float)(ofGetElapsedTimeMillis()%2000) /1000.0f;
             
             float x = (progress<=1) ? (ofMap(progress, 0, 1, rect.getLeft(), rect.getRight())) : (ofMap(progress, 2, 1, rect.getLeft(), rect.getRight()));
             
             shapes.push_back(new Line( ofPoint(x,rect.getTop()), ofPoint(x, rect.getBottom()), ofColor::white, OFXLASER_PROFILE_DEFAULT));

         }
         return shapes;
        
    }
    
};
}
