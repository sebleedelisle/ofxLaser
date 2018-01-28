ofxLaser
=========

An openFrameworks addon for rendering shapes with a standard iLDA laser projector. Currently works with the Etherdream laser controller, but more DACs to be added in the future.

Features
----------
* Can draw any vector shape to the laser with a simple function calls
* Shapes are sorted to find the optimal path for the laser to move
* Specify laser speed and acceleration for each shape using one of three "render profiles"
* Output warping to compensate for perspective distortion
* Multiple zones can be sent to multiple projectors and individually warped for mapping onto separate planes
* Colour calibration system to compensate for laser power to brightness curves
* Full rewrite of the Etherdream library - works with multiple lasers (with theoretical limits on CPU and network speeds)
* Shapes take into account the current transformation matrix, so should work with ofTransform, ofRotate, and ofScale
* Should work with 3D coordinates but needs further testing
* Cross platform - mostly tested on OSX, but should work on Windows and Linux

Roadmap
-----------
* Automatic discovery of Etherdreams (currently requires the DAC IP address)
* Masking of individual areas with varying levels
* Optimisation of point generation and sending to DACs
* IDN Ilda Digital Network support

Licence
-------
The code in this repository is available under the [MIT License](https://secure.wikimedia.org/wikipedia/en/wiki/Mit_license).  
Copyright (c) 20012-2018 Seb Lee-Delisle [seb.ly](http://seb.ly)

Installation
------------
Copy to your openFrameworks/addons folder.


Dependencies
------------
ofxXmlSettings (comes with oF)
ofxGui (comes with oF)
ofxOpenCv (comes with oF)

Compatibility
------------
openFrameworks 0098 - currently tested with 0098


Known issues
------------

Version history
------------

