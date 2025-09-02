![ofxLaser logo](https://github.com/sebleedelisle/ofxLaser/blob/main/github_preview.png?raw=true)

ofxLaser
=========

An openFrameworks addon for controlling one or more ILDA lasers, it's particularly good at rendering graphics. It currently works with Etherdream, Helios, Laserdock/LaserCube, but more DACs to be added in the future.

Comprehensive multi-laser support, limited only by your CPU. 

[![Seb Lee showreel](https://github.com/sebleedelisle/ofxLaser/blob/main/video_thumbnail.png?raw=true)](https://www.youtube.com/watch?v=l2lMqerYE78 "ofxLaser showreel")

Every project in the [showreel](https://www.youtube.com/watch?v=l2lMqerYE78 "ofxLaser showreel") was made with ofxLaser and openFrameworks. 

Safety Notice
=============
**LASERS ARE DANGEROUS - USE THIS SOFTWARE AT YOUR OWN RISK. NEVER LOOK INTO THE BEAM.** Always have an emergency stop button at hand and do not let anyone enter the laser exposure area. Check the exposure area for reflective surfaces. Take laser safety training and get licenced to use high power lasers in your location. In the UK I can recommend [this one](https://www.lvroptical.com/display.html). 

Seriously, don't mess around with this stuff. You can really damage your eyes.  

Major Update May 2021
==================

Version **1.0 beta** is now available. This is a significant upgrade from previous versions. 
* Advanced GUI that uses DearImGui
* All lasers, zones and masks are set up within compiled app's UI and saved to config files
* Much simpler set up in code, see examples
* Automatic discovery of laser controllers (DACs), with support for Ether Dream, Helios, and LaserCube / LaserDock

API changes
* No laser set up required in code any more. laser.setup(), laser.initGui no longer needed. 
* Terminology has been changed from "projector" to "laser" to avoid confusing with newcomers used to working with conventional projectors
* Adding lasers, assigning zones and editing masks is now all done within the app's GUI. No need for laser.addProjector(), laser.addZone(), laser.addZoneToProjector(), etc. 

Summary
-----------

The system is primarily designed to render graphics to one or more lasers. Conceptually we have an input canvas, and we draw everything into that. Within the canvas we can define one or more input zones that can be assigned to one or more lasers. The output for the zones can be adjusted for size and perspective. 

The default size of the canvas is 800,800 but you can change it using ofxLaser::Manager::setCanvasSize(width, height). 

ofxLaser features
----------
* Can draw any vector shape to lasers with simple function calls
* Shapes are sorted to find the optimal path for the laser
* Automatic laser controller detection
* Comprehensive multiple laser support (has been tested with 16 lasers and is limited only by CPU and network speed)
* Masking system to provide blank areas within the laser output 
* Many calibration options for blanking - colour change shift, pre/post blanks, pre/post on points
* Specify laser speed and acceleration for each shape using "render profiles" 
* Output zone transformation for projection mapping and to compensate for perspective distortion
* Multiple zones can be sent to multiple projectors and individually warped for mapping onto separate planes
* Colour calibration system to compensate for laser power to brightness curves
* Full rewrite of the Ether Dream library using Poco sockets, very reliable
* Shapes take into account the current transformation matrix, so works with ofTransform, ofRotate, and ofScale
* Works with 3D co-ordinates and shapes
* Cross platform - developed on OSX, but also tested on Windows and Linux


Usage 
-------
All API calls are via the ofxLaser::Manager object. Declare one in ofApp.h and it will be automatically initialised. 

ofxLaser::Manager::update()  - update and prepare the system to receive new laser graphics - call this in ofApp::update()

In ofApp::draw() you call all your ofxLaser draw methods (see below). Once you're finished call ofxLaser::Manager::send() to send everything to the laser(s). 

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

**NB you can use ofTranslate, ofScale and any of the built-in transformations with all of the draw methods **

Drawing methods
--------------------
### drawLine(start,  end, colour, profile);
Draws a line to the laser(s). 
returns : void
###### parameters : 
* start : *glm::vec2, glm::vec3 or ofPoint* - the start position of the line
* end : *glm::vec2, glm::vec3 or ofPoint* - the end position of the line
* colour : *ofColor* the colour of the line
* profile : (optional) the render profile, use one of the profile definitions (defaults to the default profile)

### drawLine( x1,  y1,  x2,  y2, colour, profile);
As above but with separate float values for the start and end coordinates of the line. 

### drawDot(position, colour, intensity, profile);
Draws a dot to the laser(s). This can also be used to make beam effects. Use the intensity to change the brightness - this changes how long the laser lingers to make the point so is more efficient than darkening the colour.  

###### parameters : 
- position : *glm::vec2, glm::vec3 or ofPoint* - the  position of the dot
- colour : *ofColor* the colour of the dot 
- intensity : (optional) *float* a unit value (0-1) defining the brightness of the dot. 
- profile : (optional) the render profile, use one of the profile defintions (defaults to the default profile)

### drawDot( x,  y, colour,  intensity, profile);
As above but with separate x and y values instead of a point object. 

 
### drawCircle(position, radius, colour, profile);
Draws a circle. 
###### parameters : 
- position : *glm::vec2, glm::vec3 or ofPoint* - the  position of the circle
- radius : *float* radius of the circle
- colour : *ofColor* the colour of the circle 
- profile : (optional) the render profile, use one of the profile defintions (defaults to the default profile)

### drawCircle(x, y, radius, colour,  profile);
As above but with separate x and y float values. 

### drawPoly(polyline, colour,  profile);
Draws an ofPolyline to the laser
###### parameters : 
- polyline : *ofPolyline&* - the polyline to draw
- colour : *ofColor* the colour of the polyline 
- profile : (optional) the render profile, use one of the profile defintions (defaults to the default profile)




Laser Graphic object
------------------------
The ofxLaser::Graphic class can be used to store multiple polylines and can also handle shape occlusion. It can also be used to load and render SVGs. To send the graphic to the laser use the drawLaserGraphic function. 

### drawLaserGraphic(graphic,  brightness, profile);
Draws an ofxLaser::Graphic 
###### parameters : 
- graphic : *ofxLaser::Graphic&* - the Graphic to draw
- brightness : *float* a unit value (0-1) to specify brightness 
- profile : (optional) the render profile, use one of the profile defintions (defaults to the default profile)



Supported Laser controllers
--------------------------
USB : 
* HeliosDAC 
* Laserdock / Lasercube

Network :
* Etherdream
* IDN ILDA Digital Network standard (alpha and needs adding to the DacAssigner)

Roadmap
-----------

* LaserCube network protocol (for the wifi cube)
* Add IDN to the DacAssigner system (needs the IDN Hello protocol added)
* Save / load laser presets (which contain scanner and colour presets)
* Save / load colour calibration presets
* Smart 3D mesh rendering, with silhouette and sharp edge detection
* Input masks - masks defined at the canvas side
* Improvements to shape sorting
* Persistence between frames, so that the laser doesn't change the draw order too dramatically (can cause slight flicker)
* Improvements to rendering complex shapes, slow down the laser path dependent of tightness of curves



Licence
-------
This project is licensed under the **ofxLaser License (Non-Commercial Share-Alike)**.\nIt may be used and modified for non-commercial purposes only.\nCommercial use requires a separate licence - contact [your email].\nSee the [LICENSE](LICENSE) file for details.
Copyright (c) 2012-2025 Seb Lee-Delisle [seblee.me](https://seblee.me) [seblee.co](https://seblee.co)

Installation
-------

The current main branch works with [openFrameworks](https://openframeworks.cc) v 0.11.x. 

* OF 0.11.x [main](https://github.com/openframeworks/openFrameworks) : use [ofxLaser/main](https://github.com/sebleedelisle/ofxLaser/) 

Once you have downloaded the openFrameworks source code, add the ofxLaser folder to the addons folder. 
Either clone the source code using git:

	> cd openFrameworks/addons/
	> git clone https://github.com/sebleedelisle/ofxLaser.git

Or download the source from GitHub [here](https://github.com/sebleedelisle/ofxLaser/archive/master.zip), unzip the folder, rename it from `ofxLaser-main` to `ofxLaser` and place it in your `openFrameworks/addons` folder.

To run the examples, import them into the project generator, create a new project, and open the project file in your IDE.

Legacy versions (no longer supported): 
* OF 0.11.x: use [ofxLaser/of_0.11.2](https://github.com/sebleedelisle/ofxLaser/tree/of_11.0.2)
* OF 0.10.x: use [ofxLaser/of_0.10.1](https://github.com/sebleedelisle/ofxLaser/tree/of_10.0.1) 
* OF 0.9.x : use [ofxLaser/of_0.9.8](https://github.com/sebleedelisle/ofxLaser/tree/of_0.9.8)

Dependencies
------------
addons : 
ofxOpenCv (comes with oF)  
ofxNetwork (comes with oF)  
ofxPoco (comes with oF)  

Open source libraries (included in source files) 
DearImGui
libusb
libsvgtiny
clipperlib
heliosdac
laserdocklib
ofxHersheyFont by Tobias Zimmer

Compatibility
------------

main branch : 
openFrameworks 0.11.x 

legacy versions (no longer supported)
openFrameworks 0.11.x 
openFrameworks 0.10.1 
openFrameworks 0.9.8
