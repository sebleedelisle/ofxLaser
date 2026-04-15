
ofxLaser Refactor 2025

At the moment ofxLaser is enormous, it's more like an app than a focused elegant laser driving library. It's been worked on since 2012 so there is a lot of legacy code in there and honestly, I wasn't very good at C++ when I started, or at least I hadn't figured out how to produce elegant coding patterns at that point. 
There is a considerable amount of interface code in there, and it's tangled up in the core engine code. I have made various attempts to untangle it in the past with varying degrees of success, but it's confusing and complicated, even to me. 
In the last few years I have used it as the backbone of my professional laser show software Liberation and the separation between what should be ofxLaser and Liberation has become blurred. A lot of development of ofxLaser has been specific to the requirements of Liberation, and that's not good for anyone. 
Frankly, I have to consider what should be part of the commercial software and what should be open for everyone to use, and right now I'm unhappy with that delineation, and the license is currently very restrictive - non-commercial share-alike. 
I would like to re-make ofxLaser so that it is more focussed on the task of reliably sending point data to laser controllers, with an elegant modular design pattern. In fact, the core data transfer code should be a C++ library with as few dependencies as possible. It should be possible to do it more or less entirely within the standard library. This core library would be responsible for discovering DACs, connecting to them, and managing the point stream. 
A second layer (also pure C++ with no oF dependencies) on top of that provides point stream generation utilities, ie conversion from vector data to a laser point stream, sorting vector objects, maybe even colour calibration. 
Then on top of that I could build ofxLaser, which is an openFrameworks wrapper for all of this, which provides the current drawing API functionality, and basic zoning system. 

So : 
 
- Core DAC driver system 
    - Discovers DACs
    - Connects to them
    - Manages point stream (I see a system similar to how oF handles audio data, calling a listener on a thread when the DAC is ready for points)
- Renderer system
    - sorts vectors to optimally render them 
    - Converts vectors to laser points
    - Colour calibration
- ofxLaser 
    - drawing API, s
