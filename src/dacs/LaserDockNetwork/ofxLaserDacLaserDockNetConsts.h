//
//  ofxLaserDacLaserDockNetConsts.h
//  example_HelloLaser
//
//  Created by Seb Lee-Delisle on 11/07/2023.
//
#pragma once
namespace ofxLaser {
class DacLaserDockNetConsts {
    public :
    
    // THANKS SO MUCH TO Sidney San Martín!
    // https://gist.github.com/s4y/0675595c2ff5734e927d68caf652e3af
    // Based on information from:
    // https://github.com/Wickedlasers/libLaserdockCore/blob/master/3rdparty/laserdocklib/src/LaserDockNetworkDevice.cpp
    // The laser is listening on at least three UDP ports and responds to unicast
    // and broadcast messages. This makes it possible to control multiple LaserCubes
    // on the network individually or all as one (by sending messages to the
    // broadcast address). However, controlling multiple cubes as one could make it
    // trickier to manage backpressure (i.e. to keep track of how much buffer is
    // free on each cube and adjust sending speed).

    // Each port listens for and responds to different categories of messages, but
    // the messages are in the same format.
    // For "alive" messages (simple pings to check which lasers are on the network).
    // This code currently just uses the GET_FULL_INFO command instead.
    static const int ALIVE_PORT = 45456;
    // For commands (get information from the laser, enable/disable output, set the
    // ILDA point rate, etc.)
    static const int CMD_PORT = 45457;
    // For data, i.e. actual points to scan out with the laser.
    static const int DATA_PORT = 45458;
    
    static const uint8_t CMD_GET_ALIVE = 0x27;
    
    // All commands are UDP messages where the first byte is the command ID and the
    // remaining bytes (if any) are specific to the command. When sending data, be
    // careful to keep each message small enough to fit inside the network's MTU;
    // more on that below. Here are the commands used in this example:
    // The laser responds with a bunch of status information; see below.
    static const uint8_t CMD_GET_FULL_INFO = 0x77;
    // Causes the laser to reply to data packets with the amount of free space left
    // in the buffer.
    static const uint8_t CMD_ENABLE_BUFFER_SIZE_RESPONSE_ON_DATA = 0x78;
    // Enables/disables output. Second byte should be zero or one. In my experience,
    // this doesn't actually disable output if you're still sending samples, but
    // disabling output *is* necessary for the menu button on the back of the
    // LaserCube to work.
    static const uint8_t CMD_SET_OUTPUT = 0x80;
    // The laser responds with a count of free space in the RX buffer.
    static const uint8_t CMD_GET_RINGBUFFER_EMPTY_SAMPLE_COUNT = 0x8a;
    // Sends a list of points to be scanned out. Must be sent to DATA_PORT. As far
    // as I can tell, the second byte of the message is always 0x0, the third and
    // fourth bytes are sequence numbers representing the message and frame, and the
    // remainder of the message is a list of points. So, you can split a frame into
    // as many messages as you want to stay within the network's MTU, but the
    // sequence number needs to go up with each one (and wrap back to zero after
    // 255). In other words, a message is structured like this:
    //     { CMD_SAMPLE_DATA, 0x00, message_number, frame_number, x, y, r, g, b, x, y, r, g, b, ... }
    // message_number should go up after every message and frame_number should go up
    // after every complete "frame". Note: x, y, r, g, b are two bytes each, with a
    // range of 0x0-0xfff (NOT 0x0-0xffff).
    static const uint8_t CMD_SAMPLE_DATA = 0xa9;
    
    static const uint8_t CMD_SET_ILDA_RATE = 0x82;
};
}
    
    
    
