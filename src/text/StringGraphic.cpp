//
//  LaserStringGraphic.cpp
//  LaserTrainTimes
//
//  Created by Seb Lee-Delisle on 07/06/2020.
//
//

#include "StringGraphic.h"

using namespace ofxLaser;

bool StringGraphic :: setString (string newString, const ofColor& newColour) {
	
	if((textString!=newString) || (colour!=newColour)) {
		textString = newString;
		colour = newColour;
		updateGraphic(); 
		return true; 
	}
	return false; 
	
	
}
void StringGraphic :: setAlignment(Alignment newalignment) {
    if(alignment!=newalignment) {
        alignment = newalignment;
        updateGraphic();
    }
}
bool StringGraphic :: setString (string newString) {
	return setString(newString, colour); 
	
}

void StringGraphic :: updateGraphic() {
    // should also release the polylines
    clear();
    
    
    
    polylineCharacters = textWriter.getPolylines(textString, glm::vec3(0,0,0), 4, true);
    
    for(vector<ofPolyline*>& polys : polylineCharacters) {
        for(ofPolyline* poly : polys)  {
            //poly->draw();
            addPolyline(poly, colour);
        }
        
        
    }
    
    for(vector<ofPolyline*>& polys : polylineCharacters) {
        for(ofPolyline* poly : polys)  {
            ofxLaser::Factory::releasePolyline(poly);
        }


    }
    polylineCharacters.clear();
//	clear();
//	textPath.clear();
//
//	hersheyFont.setColor(colour);
//	textPath = hersheyFont.getPath(textString,1);
//	//ofPushMatrix();
//	if(alignment == ALIGN_RIGHT) {
////		ofTranslate(-getWidth(), 0);
//        textPath.translate(glm::vec2(-getWidth(), 0));
//	} else if(alignment == ALIGN_CENTRE) {
//		//ofTranslate(-getWidth()/2, 0);
//        textPath.translate(glm::vec2(-getWidth()/2, 0));
//	}
//
//	addPath(textPath, false, false);
	
}

float StringGraphic :: getWidth() {
	return hersheyFont.getWidth(textString, 1);
}
