ofxLaser
=========

An openFrameworks addon for controlling one or more ILDA lasers, it's particularly good at rendering graphics. It currently works with Etherdream, Helios, Laserdock/LaserCube, but more DACs to be added in the future.

Safety Notice
=============
**LASERS ARE DANGEROUS - USE THIS SOFTWARE AT YOUR OWN RISK. NEVER LOOK INTO THE BEAM.** Always have an emergency stop button at hand and do not let anyone enter the laser exposure area. Check the exposure area for reflective surfaces. Take laser safety training and get licenced to use high power lasers in your location. In the UK I can recommend [this one](https://www.lvroptical.com/display.html). 

Seriously, don't mess around with this stuff. You can really damage your eyes.  

Major Update May 2021
==================

Version **1.0 beta** is now available. This is a significant upgrade from previous versions. 
* Advanced GUI that uses DearIMGui
* All laser set up is done within the compiled app and saved to config files
* Much simpler set up in code, see examples
* Automatic detection of laser controllers (DACs), including etherdream, Helios, and LaserCube

API changes
* No laser set up required in code any more. laser.setup(), laser.initGui no longer needed. 
* Terminology has been changed from "projector" to "laser" to avoid confusing with newcomers used to working with conventional projectors
* Adding lasers, assigning zones and editing masks is now all done within the app's GUI. No need for laser.addProjector(), laser.addZone(), laser.addZoneToProjector(), etc. 


Usage 
-------
All API calls are via the ofxLaser::Manager object. 

ofxLaser::Manager::update()  - update and prepare the system to receive new laser graphics
ofxLaser::Manager::send() - send everything drawn this time to the lasers
ofxLaser::Manager::drawUI() - draw the laser UI system

Note that all drawing methods must be called after update() and before send(). 

Drawing to the laser 
--------------------
We have three render profiles for each laser, default, fast and detailed, referenced using the following compiler definitions:
OFXLASER_PROFILE_DEFAULT
OFXLASER_PROFILE_FAST
OFXLASER_PROFILE_DETAIL

Use the default profile for most things, use fast for any smooth lines where accuracy doesn't matter, and detail for anything intricate. Bear in mind that the output will get flickery the more detailed you get. 

These three profiles can be edited within the app's GUI, and can be defined differently for each laser. You can also save and load scanner presets. 

Drawing methods
--------------------
##### drawLine(start,  end, colour, profile);
Draws a line to the laser(s). 
returns : void
###### parameters : 
* start : *glm::vec2, glm::vec3 or ofPoint* - the start position of the line
* end : *glm::vec2, glm::vec3 or ofPoint* - the end position of the line
* colour : *ofColor* the colour of the line
* profile : (optional) the render profile, use one of the profile definitions (defaults to the default profile)

##### drawLine( x1,  y1,  x2,  y2, colour, profile);
Draws a line to the laser(s). 
returns : void
###### parameters : 
- x1, y1, x2, y2 : *floats*, start and end coordinates for the line. 
- colour :  *ofColor* the colour of the line
- profile : (optional) the render profile, use one of the profile defintions (defaults to the default profile)

##### drawDot(position, colour, intensity, profile);
Draws a dot to the laser(s). This can also be used to make beam effects. Use the intensity to change the brightness - this changes how long the laser lingers to make the point so is more efficient than darkening the colour.  
returns : void
###### parameters : 
- position : *glm::vec2, glm::vec3 or ofPoint* - the  position of the dot
- colour : *ofColor* the colour of the dot 
- intensity : (optional) *float* a unit value (0-1) defining the brightness of the dot. 
- profile : (optional) the render profile, use one of the profile defintions (defaults to the default profile)

##### drawDot( x,  y, colour,  intensity, profile);
As above but with separate x and y values instead of a point object. 
returns : void
###### parameters : 
- x, y : *floats* - the x and y position of the dot
- colour : *ofColor* the colour of the dot 
- intensity : (optional) *float* a unit value (0-1) defining the brightness of the dot. 
- profile : (optional) the render profile, use one of the profile defintions (defaults to the default profile)
    
drawCircle(x, const float& y, const float& radius,const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);
drawCircle(const glm::vec3& centre, const float& radius,const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);
drawCircle(const glm::vec2& centre, const float& radius,const ofColor& col, string profileName= OFXLASER_PROFILE_DEFAULT);

drawPoly(const ofPolyline &poly, const ofColor& col,  string profileName = OFXLASER_PROFILE_DEFAULT);
drawPoly(const ofPolyline & poly, vector<ofColor>& colours, string profileName = OFXLASER_PROFILE_DEFAULT);

Laser Graphic object
------------------------
The ofxLaser::Graphic class can be used to store multiple polylines and can also handle shape occlusion. It can also be used to load and render SVGs. To send the graphic to the laser use the drawLaserGraphic function. 

void drawLaserGraphic(Graphic& graphic, float brightness = 1, string renderProfile = OFXLASER_PROFILE_DEFAULT);





Features
----------
* Can draw any vector shape to lasers with simple function calls
* Shapes are sorted in real-time to find the optimal path for the laser
* Automatic laser controller detection
* Can control multiple lasers (has been tested with 16 lasers and is limited only by CPU and network speed)
* Masking of individual areas with varying levels 
* Many calibration options for blanking - colour change offset, pre/post blanks, pre/post on points
* Specify laser speed and acceleration for each shape using "render profiles" 
* Output zone transformation for projection mapping and to compensate for perspective distortion
* Multiple zones can be sent to multiple projectors and individually warped for mapping onto separate planes
* Colour calibration system to compensate for laser power to brightness curves
* Full rewrite of the Etherdream library using Poco sockets, very reliable
* Shapes take into account the current transformation matrix, so works with ofTransform, ofRotate, and ofScale
* Works with 3D coordinates and shapes
* Cross platform - developed on OSX, but also tested on Windows and Linux

Supported Laser controllers
--------------------------
USB : 
* HeliosDAC 
* Laserdock / Lasercube

Network :
* Etherdream
* IDN ILDA Digital Network standard (alpha)

Roadmap
-----------

Licence
-------
The code in this repository is available under the [MIT License](https://secure.wikimedia.org/wikipedia/en/wiki/Mit_license).  
Copyright (c) 2012-2019 Seb Lee-Delisle [seblee.me](https://seblee.me) [seblee.co](https://seblee.co)

Installation
-------

First, pick the branch that matches your version of openFrameworks:

* OF 0.11.x [main](https://github.com/openframeworks/openFrameworks) : use [ofxLaser/main](https://github.com/sebleedelisle/ofxLaser/) 
* OF 0.10.x: use [ofxLaser/of_0.10.1](https://github.com/sebleedelisle/ofxLaser/tree/of_9.10.1) 
* OF 0.9.x : use [ofxLaser/of_0.9.8](https://github.com/sebleedelisle/ofxLaser/tree/of_0.9.8)

Either clone the source code using git:

	> cd openFrameworks/addons/
	> git clone https://github.com/sebleedelisle/ofxLaser.git

Or download the source from GitHub [here](https://github.com/sebleedelisle/ofxLaser/archive/master.zip), unzip the folder, rename it from `ofxLaser-main` to `ofxLaser` and place it in your `openFrameworks/addons` folder.

To run the examples, import them into the project generator, create a new project, and open the project file in your IDE.


Dependencies
------------
ofxOpenCv (comes with oF)  
ofxNetwork (comes with oF)  
ofxPoco (comes with oF)  
ofxSvg (comes with oF)  

Compatibility
------------
openFrameworks 0.11.x
openFrameworks 0.10.1 
openFrameworks 0.9.8

Known issues
------------

Version history
------------

