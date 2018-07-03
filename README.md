ofxLaser
=========

An openFrameworks addon for controlling one or more ILDA lasers, it's particularly good at rendering graphics. It currently works with the Etherdream and IDN controllers, but more DACs to be added in the future.

Safety Notice
=============
**LASERS ARE DANGEROUS - USE THIS SOFTWARE AT YOUR OWN RISK. NEVER LOOK INTO THE BEAM.** Always have an emergency stop button at hand and do not let anyone enter the laser exposure area. Check the exposure area for reflective surfaces. Take laser safety training and get licenced to use high power lasers in your location. In the UK I can recommend [this one](https://www.lvroptical.com/display.html). 

Seriously, don't fuck around with this stuff. You can really damage your eyes. You've been warned. 

Features
----------
* Can draw any vector shape to the laser with a simple function calls
* Shapes are sorted in real-time to find the optimal path for the laser
* Can control multiple lasers (limited only by CPU and network speed)
* Masking of individual areas with varying levels at source
* Many calibration options for blanking - colour change offset, pre/post blanks, pre/post on points
* Specify laser speed and acceleration for each shape using "render profiles" 
* Output warping to compensate for perspective distortion
* Multiple zones can be sent to multiple projectors and individually warped for mapping onto separate planes
* Colour calibration system to compensate for laser power to brightness curves
* Full rewrite of the Etherdream library using Poco sockets, very reliable
* Shapes take into account the current transformation matrix, so should work with ofTransform, ofRotate, and ofScale
* Should work with 3D coordinates but needs further testing
* Cross platform - mostly tested on OSX, but should work on Windows and Linux

Roadmap
-----------
* Automatic discovery of Etherdreams (currently requires the DAC IP address)
* Automatic discovery of IDN DACs (currently requires the DAC IP address)
* Masking in laser space, not source space
* Optimisation of point generation and sending to DACs

Licence
-------
The code in this repository is available under the [MIT License](https://secure.wikimedia.org/wikipedia/en/wiki/Mit_license).  
Copyright (c) 20012-2018 Seb Lee-Delisle [seb.ly](http://seb.ly)

Installation
-------

First, pick the branch that matches your version of openFrameworks:

* OF [stable](https://github.com/openframeworks/openFrameworks/tree/stable) (0.9.8): use [ofxCv/stable](https://github.com/sebleedelisle/ofxLaser/tree/stable)
* OF [master](https://github.com/openframeworks/openFrameworks) (0.10.0): use [ofxCv/master](https://github.com/sebleedelisle/ofxLaser/)

Either clone out the source code using git:

	> cd openFrameworks/addons/
	> git clone https://github.com/sebleedelisle/ofxLaser.git

Or download the source from GitHub [here](https://github.com/sebleedelisle/ofxLaser/archive/master.zip), unzip the folder, rename it from `ofxLaser-master` to `ofxLaser` and place it in your `openFrameworks/addons` folder.

To run the examples, import them into the project generator, create a new project, and open the project file in your IDE.


Dependencies
------------
ofxXmlSettings (comes with oF)
ofxGui (comes with oF)
ofxOpenCv (comes with oF)
ofxNetwork (comes with oF)

Known issues
------------

Version history
------------

