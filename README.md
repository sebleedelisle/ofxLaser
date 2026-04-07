![ofxLaser logo](https://github.com/sebleedelisle/ofxLaser/blob/main/github_preview.png?raw=true)

ofxLaser
=========

[![Build Examples](https://github.com/sebleedelisle/ofxLaser/actions/workflows/example-smoke-check.yml/badge.svg?branch=libera-major-refactor)](https://github.com/sebleedelisle/ofxLaser/actions/workflows/example-smoke-check.yml)

ofxLaser is a core library for sending laser frames to supported laser controllers (DACs) (Ether Dream, Helios, LaserCube, AVB, etc).  

It's built on [libera-laser](https://github.com/sebleedelisle/libera-laser) which provides all the low level discovery, communication, and frame queuing functionality, while ofxLaser adds an openFrameworks wrapper, along with complex point stream generation, colour correction and zone handling. 

This library is intended for developers, educators, and hobbyists who want to experiment with laser control in openFrameworks.  
It is licensed for **non-commercial use only** (see [LICENSE](LICENSE)).

---

## Looking for complete show software?

If you need a full professional environment for designing and running shows, check out **[Liberation](https://liberationlaser.com)**.  

Liberation is built on the same foundation as ofxLaser, but adds:

- A 3D visualiser and advanced editing tools
- Powerful node editing tool for creating laser graphics and beam effects
- Clip deck, timeline, and FX systems  
- MIDI/DMX/Art-Net integration  
- PONK support for integration with TouchDesigner, MadMapper and openFrameworks
- Professional support and reliability  

Liberation is the **polished, supported application**, while ofxLaser remains a lean toolkit for learning and prototyping.

Safety Notice
=============
**LASERS ARE DANGEROUS - USE THIS SOFTWARE AT YOUR OWN RISK. NEVER LOOK INTO THE BEAM.** Always have an emergency stop button at hand and do not let anyone enter the laser exposure area. Check the exposure area for reflective surfaces. Take laser safety training and get licenced to use high power lasers in your location. In the UK I can recommend [this one](https://www.lvroptical.com/display.html). 

Seriously, don't mess around with this stuff. You can really damage your eyes / burn your house down.  

Current status
==================

The **libera-major-refactor** branch is undergoing a significant overhaul:
* Pure ofxGui interface (ImGui dependency removed)
* Poco dependency removed
* DAC communication handled by **libera-core** backend
* All lasers, zones and masks are set up within the app's UI and saved to config files
* Automatic discovery of laser controllers (DACs)
* Much simpler set up in code, see examples

Summary
-----------

The system is primarily designed to render graphics to one or more lasers. Conceptually we have an input canvas, and we draw everything into that. Within the canvas we can define one or more input zones that can be assigned to one or more lasers. The output for the zones can be adjusted for size and perspective. 


ofxLaser features
----------
* Can draw any vector shape to lasers with simple function calls
* Shapes are sorted to find the optimal path for the laser
* Automatic laser controller detection (using libera)
* Masking system to provide blank areas within the laser output 
* Many calibration options for blanking - colour change shift, pre/post blanks, pre/post on points
* Specify laser speed and acceleration for each shape using "render profiles" 
* Output zone transformation for projection mapping and to compensate for perspective distortion
* Multiple zones can be sent to multiple projectors and individually warped for mapping onto separate planes
* Colour calibration system to compensate for laser power to brightness curves
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
* LaserCube USB

Network :
* Ether Dream
* LaserCube (WiFi)
* IDN (ILDA Digital Network)
* AVB (Audio Video Bridging)


Licence
-------
This project is licensed under the **ofxLaser License (Non-Commercial Share-Alike)**.\nIt may be used and modified for non-commercial purposes only.\nCommercial use requires a separate licence - contact [seb at seblee.co].\nSee the [LICENSE](LICENSE) file for details.
Copyright (c) 2012-2026 Seb Lee-Delisle [seblee.me](https://seblee.me) [seblee.co](https://seblee.co)

Installation
-------

The **libera-major-refactor** branch targets [openFrameworks](https://openframeworks.cc) 0.12.x.

Once you have downloaded the openFrameworks source code, add the ofxLaser folder to the addons folder. 
Either clone the source code using git:

	> cd openFrameworks/addons/
	> git clone https://github.com/sebleedelisle/ofxLaser.git

Or download the source from GitHub [here](https://github.com/sebleedelisle/ofxLaser/archive/master.zip), unzip the folder, rename it from `ofxLaser-main` to `ofxLaser` and place it in your `openFrameworks/addons` folder.

To run the examples, import them into the project generator, create a new project, and open the project file in your IDE.

Dependencies
------------
addons : 
ofxGui (comes with oF)  

Open source libraries (included in source) :
libera-core
clipper
libusb
ofxSvgExtra


Example smoke check
-------------------

To compile every example without relying on checked-in project files, run:

```sh
./scripts/smoke-check-examples.sh
```

The script builds openFrameworks once in `Release`, generates temporary
makefile-based wrappers for each `example_*` folder, and then compiles each
example in turn.

Compatibility
------------

openFrameworks 0.12.x 


