//
//  Asteroids.cpp
//  
//
//  Created by Seb Lee-Delisle on 29/05/2016.
//
//

#include "Asteroids.h"

Asteroids :: Asteroids() {
	
	gameState = STATE_WAITING;
	timeSinceStateChange = 0;
	firePressed = false;
	leftKeyDown = false;
	rightKeyDown = false;
	upKeyDown = false;
	
	gameWidth = 800;
	gameHeight = 600;
	gameRect.set(0,0,800,600);
	reset();
	
	laserSound.load("fire.wav");
	bangSmall.load("bangSmall.wav");
	bangMedium.load("bangMedium.wav");
	bangLarge.load("bangLarge.wav");
    
	ofAddListener(ofEvents().keyPressed, this, &Asteroids::keyPressed);
	ofAddListener(ofEvents().keyReleased, this, &Asteroids::keyReleased);
	
	clickOn.load("clickon.wav");
	clickOff.load("clickoff.wav");
	
	textWriter.lineRandomness = 0;
	textWriter.colourFlickerMin = 1;
	
	active = false; 
	
};

void Asteroids :: update(float deltaTime){
	
	timeSinceStateChange+=deltaTime;

	// interactive stuff if we're currently playing the game
	if(gameState == STATE_PLAYING) {
		if(leftKeyDown) ship.rotateLeft(deltaTime);
		if(rightKeyDown) ship.rotateRight(deltaTime);
		if(upKeyDown) ship.thrust(deltaTime);
		
		
	
		bool bulletsinside = false;
		for(int i = 0; i<bullets.size(); i++) {
			bullets[i].update(deltaTime);
			if(gameRect.inside(bullets[i].pos)) bulletsinside = true;
		}
		if(!bulletsinside) bullets.clear();
		
		updateAsteroids(deltaTime);
		
		checkCollisions();
	} else {
		updateAsteroids(deltaTime);
	}
	
	ship.update(deltaTime, gameRect);
	
	
	// update particles :
	for(int i = 0; i<particles.size(); i++) {
		particles[i].update(deltaTime);
		particles[i].vel*=0.9;
		
		if(particles[i].lifeTime>0.5) {
			particles.erase(particles.begin()+i);
			i--;
		}
	}
	
	if((gameState == STATE_WAITING) || (gameState == STATE_GAME_OVER)) {
		if((firePressed) && (timeSinceStateChange>2)) {
			startGame();
		}
	} else if (gameState == STATE_RESTART_WAIT) {
		if(timeSinceStateChange>2) {
			ship.resetPosition();
			if(isSafe()) {
				changeState(STATE_PLAYING);
				ship.reset();
			}
		}
	}
	
	
};


void Asteroids ::draw(ofxLaser::Manager &lm) {
	
	for(int i = 0; i<asteroids.size(); i++) {
		asteroids[i].draw(lm);
	}
	for(int i = 0; i<bullets.size(); i++) {
		lm.addLaserDot(bullets[i].pos, ofColor::white);
	}
	for(int i = 0; i<particles.size(); i++) {
		lm.addLaserDot(particles[i].pos, ofColor::white,0.02);
	}

	//if(gameState == STATE_PLAYING) {
		ship.draw(lm);
	//}
	
	if((gameState == STATE_WAITING) || (gameState == STATE_GAME_OVER)) {
		
		textWriter.colour = ofColor::white;
		string text = gameState==STATE_GAME_OVER ? "GAME OVER" : "ASTEROIDS";
		
		ofMesh laserWordMesh = textWriter.getMesh(text, ofPoint(400,250), 6, true);
		
		//ofPolyline poly;
		vector<ofVec3f>& vertices = laserWordMesh.getVertices();

		for(int i = 0; i<vertices.size(); i+=2) {
			if(i+1>=vertices.size()) break;
			lm.addLaserLine(vertices[i], vertices[i+1], laserWordMesh.getColors()[i]);
		}
	
		
	}
	
	for(int i = 0; i<lives;i++) {
		lm.addLaserPolyline(ship.shipImage, ofColor::white, ofPoint(50+(20*i),50),-90,ofPoint(0.9,0.9));
	}
	
	
	textWriter.colour = ofColor::white;
	
	ofMesh laserWordMesh = textWriter.getMesh(ofToString(score), ofPoint(400,50), 4, true);
	
	//ofPolyline poly;
	vector<ofVec3f>& vertices = laserWordMesh.getVertices();
	
	for(int i = 0; i<vertices.size(); i+=2) {
		if(i+1>=vertices.size()) break;
		lm.addLaserLine(vertices[i], vertices[i+1], laserWordMesh.getColors()[i]);
	}

	
}

void Asteroids::updateAsteroids(float deltaTime) {
	// always update the asteroids no matter what
	for(int i = 0; i<asteroids.size(); i++) {
		
		Asteroid& asteroid = asteroids[i];
		asteroid.update(deltaTime);
		if(asteroid.pos.x-asteroid.radius>gameWidth) {
			asteroid.pos.x = -asteroid.radius;
		} else if(asteroid.pos.x+asteroid.radius<0) {
			asteroid.pos.x = gameWidth+asteroid.radius;
		}
		
		
		if(asteroid.pos.y-asteroid.radius>gameHeight) {
			asteroid.pos.y = -asteroid.radius;
		} else if(asteroid.pos.y+asteroid.radius<0) {
			asteroid.pos.y = gameHeight+asteroid.radius;
		}
		
	}

}

void Asteroids::checkCollisions(){
	deque<Asteroid> newasteroids;
	
	for (int i = 0; i<asteroids.size(); i++) {
		
		Asteroid& asteroid = asteroids[i];
		
		//ofLog(OF_LOG_NOTICE, ofToString(ship.pos.distance(asteroid.pos)));
		// first, check if the asteroid has hit the ship
		if(ship.pos.distance(asteroid.pos) < asteroid.radius+12) {
			// ship has exploded!
			ship.explode();
			lives--;
			bullets.clear();
			if(lives>=0)
				changeState(STATE_RESTART_WAIT);
			else
				changeState(STATE_GAME_OVER);
			break;
		}
		
		// now check the asteroid with every bullet
		for(int j = 0; j<bullets.size(); j++) {
			

			Bullet& bullet = bullets[j];
			
			// if the bullet is too close to the asteroid, then
			// we have a collision
			
			if(bullet.pos.distance(asteroid.pos) < asteroid.radius) {
				
				makeParticleExplosion(asteroid.pos);
				if(asteroid.radius==50) {
					score+=20;
				} else if(asteroid.radius==25) {
					score+=50;
				} else if(asteroid.radius==12.5) {
					score+=100;
				}
				ofLog(OF_LOG_NOTICE, ofToString(asteroid.radius) + " " + ofToString(score));
				
				// if the asteroid is big, then halve its size
				if(asteroid.radius>15) {
					
					// and make a new asteroid in the same position
					// add it on to the array
					
					Asteroid newasteroid;
					newasteroid.pos.set(asteroid.pos);
					asteroid.setRadius(asteroid.radius/2);
					asteroid.vel*=1.8;
					newasteroid.vel = asteroid.vel;
					newasteroid.vel.rotate(ofRandom(-90,90),ofPoint(0,0,1));
					//asteroid.vel.rotate(20,ofPoint(0,0,1));
					
					newasteroid.setRadius(asteroid.radius);
					newasteroids.push_back(newasteroid);
					
					bangLarge.play();
					
				} else {
					// otherwise the asteroid is small, so we just destroy it
					asteroids.erase(asteroids.begin()+i);
					i--; // don't strictly need this as we're breaking out of this loop anyway
					bangSmall.play();
				}
				// and then delete the bullet
				bullets.erase(bullets.begin()+j);
				j--;
				break;
			}
		}
	}
	
	for(int i = 0; i<newasteroids.size(); i++) {
		asteroids.push_back(newasteroids[i]);
	}
	
	if(asteroids.size()==0) {
		reset();
		ship.exploding = true;
		ship.explodeProgress = 2;
		changeState(STATE_RESTART_WAIT);
	}
}


void Asteroids ::fire() {
	if(gameState!=STATE_PLAYING) return; 
	bullets.push_back(Bullet(ship.pos,ship.rotation));
	if(bullets.size()>10) bullets.pop_front();
	laserSound.play();
}

void Asteroids :: changeState (GameState newState){
	gameState = newState;
	timeSinceStateChange = 0;
	
}

void Asteroids ::reset() {
	
	ship.resetPosition();
	asteroids.clear();
	
	for (int i = 0; i<4; i++) {
		
		Asteroid asteroid;
		asteroid.pos.set(ofRandom(gameWidth),ofRandom(gameHeight));
		asteroids.push_back(asteroid);
		
	}
	
}

void Asteroids :: startGame() {
	reset();
	score = 0;
	lives = 3;
	changeState(STATE_RESTART_WAIT);
	
}

void Asteroids ::makeParticleExplosion(ofPoint pos) {
	
	for(int i = 0; i<12; i++) {
		particles.push_back(Bullet(pos,ofRandom(0,360)));
		particles.back().vel*=ofRandom(0,0.5);
	}
}



//--------------------------------------------------------------
void Asteroids::keyPressed(ofKeyEventArgs &e){
	
	if(!active) return;
	
	if ((e.key == OF_KEY_LEFT) && (!leftKeyDown)) {
		leftKeyDown = true;
	} else if ((e.key == OF_KEY_RIGHT) && (!rightKeyDown)) {
		rightKeyDown = true;
	} else if ((e.key == OF_KEY_UP) && (!upKeyDown)) {
		upKeyDown = true;
	} else if((e.key==' ') && !firePressed) {
		fire();
		firePressed = true;
	} else return;
	
	clickOn.setSpeed(ofRandom(0.95,1.05));
	clickOn.play();
}

//--------------------------------------------------------------
void Asteroids::keyReleased(ofKeyEventArgs &e){
	
	if (e.key == OF_KEY_LEFT) {
		leftKeyDown = false;
	} else if (e.key == OF_KEY_RIGHT) {
		rightKeyDown = false;
	} else if (e.key == OF_KEY_UP) {
		upKeyDown = false;
	} else if(e.key==' ') {
		firePressed = false;
	} else return;
	
	if(!active) return;
	
	clickOff.play();
}

bool Asteroids:: isSafe() {
	
	for (int i = 0; i<asteroids.size(); i++) {
		Asteroid& asteroid = asteroids[i];
		if(ship.pos.distance(asteroid.pos) < asteroid.radius+50) {
			return false;
		}
	}
	return true;
	
}
