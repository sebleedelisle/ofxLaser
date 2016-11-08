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
	
	gameWidth = 800;
	gameHeight = 600;
	gameRect.set(0,0,800,600);
	reset();
	
	//ship.addVertex(14,0);
	ship.addVertex(-8,-8);
	ship.lineTo(-5,-5);
	ship.lineTo(-5,5);
	ship.lineTo(-8,8);
	ship.lineTo(14,0);
	ship.lineTo(-8,-8);
	
	laserSound.loadSound("fire.wav");
	bangSmall.loadSound("bangSmall.wav");
	bangMedium.loadSound("bangMedium.wav");
	bangLarge.loadSound("bangLarge.wav");
	
};

void Asteroids :: update(float deltaTime, bool rotateLeft, bool rotateRight){
	
	timeSinceStateChange+=deltaTime;

	// interactive stuff if we're currently playing the game
	if(gameState == STATE_PLAYING) {
		if(rotateLeft) shipRotation-=6*deltaTime*50;
		if(rotateRight) shipRotation+=6*deltaTime*50;
	
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
		//if(KeyTracker.isKeyDown(Key.SPACE)) {
		//	startGame();
		//}
	} else if (gameState == STATE_RESTART_WAIT) {
		//if(framesSinceStateChange>60) {
		//	resetShip();
		//	if(isSafe()) changeState(STATE_PLAYING);
		//}
	}
	
	
};

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
		
		
		// now check the asteroid with every bullet
		for(int j = 0; j<bullets.size(); j++) {
			
			Bullet& bullet = bullets[j];
			Asteroid& asteroid = asteroids[i];
			
			
			// if the bullet is too close to the asteroid, then
			// we have a collision
			
			if(bullet.pos.distance(asteroid.pos) < asteroid.radius) {
				
				makeParticleExplosion(asteroid.pos);
				
				// if the asteroid is big, then halve its size
				if(asteroid.radius>15) {
					
					// and make a new asteroid in the same position
					// add it on to the array
					
					Asteroid newasteroid;
					newasteroid.pos.set(asteroid.pos);
					asteroid.setRadius(asteroid.radius/2);
					asteroid.vel*=1.6;
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
}
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
	// update ship
	ofPolyline tship;
	transformPolyline(ship, tship, ofPoint(400,300), shipRotation, ofPoint(1,1));
	lm.addLaserPolyline(tship, ofColor::white);
}


void Asteroids ::fire() {
	if(gameState!=STATE_PLAYING) return; 
	bullets.push_back(Bullet(ofPoint(400,300),shipRotation));
	if(bullets.size()>10) bullets.pop_front();
	laserSound.play();
}

void Asteroids :: changeState (GameState newState){
	gameState = newState;
	timeSinceStateChange = 0;
	
}

void Asteroids ::reset() {
	asteroids.clear();
	
	shipRotation = 0;
	
	for (int i = 0; i<4; i++) {
		
		Asteroid asteroid;
		asteroid.pos.set(ofRandom(gameWidth),ofRandom(gameHeight));
		asteroids.push_back(asteroid);
		
	}
	
}

void Asteroids ::transformPolyline(ofPolyline& source, ofPolyline& target, ofPoint pos, float rotation, ofPoint scale) {
	
	target = source;
	for(int i = 0; i<target.size(); i++) {
		ofPoint& p = target[i];
		p*=scale;
		p.rotate(rotation, ofPoint(0,0,1));
		p+=pos;
	}
}

void Asteroids ::makeParticleExplosion(ofPoint pos) {
	
	for(int i = 0; i<12; i++) {
		particles.push_back(Bullet(pos,ofRandom(0,360)));
		particles.back().vel*=ofRandom(0,0.5);
	}
}

