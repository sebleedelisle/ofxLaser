//
//  Asteroids.h
//  example_Games
//
//  Created by Seb Lee-Delisle on 14/05/2016.
//
//

#pragma once

#include "ofxLaserManager.h"
#include "ofMain.h"
#include "Asteroid.h"
#include "Bullet.h"
#include "Ship.h"
#include "TextWriter.h"

class Asteroids {

	public :
	
	enum GameState{
		STATE_WAITING,
		STATE_PLAYING,
		STATE_RESTART_WAIT,
		STATE_GAME_OVER
	} gameState;
	
	Asteroids();
	
	void update(float deltaTime);
	
	void draw(ofxLaser::Manager &lm);

    void keyPressed(ofKeyEventArgs &e);
    void keyReleased(ofKeyEventArgs &e);
    
	void fire();
	void reset();
	void startGame();
	
	void updateAsteroids(float deltaTime);
	void checkCollisions();
	
	void changeState (GameState newState);

	vector<Asteroid> asteroids;
	int gameWidth, gameHeight;
	ofRectangle gameRect;
	
	bool active;
	
	Ship ship;

	protected :
	
	void makeParticleExplosion(ofPoint pos);
	bool isSafe();
	
	deque<Bullet> bullets;
	deque<Bullet> particles;
	int score = 0;
	int lives = 3;
	
	ofSoundPlayer laserSound;
	ofSoundPlayer bangSmall;
	ofSoundPlayer bangMedium;
	ofSoundPlayer bangLarge;
	
	
	ofSoundPlayer clickOn;
	ofSoundPlayer clickOff;
	
	bool leftKeyDown;
	bool rightKeyDown;
	bool upKeyDown;
	bool firePressed;
	
	float timeSinceStateChange;
	
	TextWriter textWriter;

};
