
// Modified from Robert Penner's easing equations
#pragma once 

class Quint {
	
public:
	
	// t = time (between 0 and d) 
	// v = start value
	// c = end value
	// d = duration
	static float easeIn (float t,float b , float c, float d) {
		t/=d;
		return c*t*t*t*t*t + b;
	}
	static float easeOut(float t,float b , float c, float d) {
		t=t/d-1;
		return c*(t*t*t*t*t + 1) + b;
	}
	
	static float easeInOut(float t, float b , float c, float d) {
		t/=d/2;
		if (t < 1) return c/2*t*t*t*t*t + b;
		t-=2;
		return c/2*(t*t*t*t*t + 2) + b;
	}
	
};


class Quart {
	
public:
	
	// t = time (between 0 and d)
	// v = start value
	// c = end value
	// d = duration
	static float easeIn (float t,float b , float c, float d) {
		t/=d;
		return c*t*t*t*t + b;
	}
	static float easeOut(float t,float b , float c, float d) {
		t=t/d-1;
		return c*(t*t*t*t + 1) + b;
	}
	
	static float easeInOut(float t, float b , float c, float d) {
		t/=d/2;
		if (t < 1) return c/2*t*t*t*t + b;
		t-=2;
		return c/2*(t*t*t*t + 2) + b;
	}
	
};

class Cubic {
	
public:
	
	// t = time (between 0 and d)
	// v = start value
	// c = end value
	// d = duration
	
	static float easeIn (float t,float b , float c, float d) {
		t/=d;
		return c*(t)*t*t + b;
	}
	static float easeOut(float t,float b , float c, float d) {
		t=t/d-1;
		return c*((t)*t*t + 1) + b;
	}
	static float easeInOut(float t,float b , float c, float d) {
		if ((t/=d/2) < 1) return c/2*t*t*t + b;
		t-=2;
		return c/2*((t)*t*t + 2) + b;
	}
	
};

class Quad {
	
public:
	
	// t = time (between 0 and d)
	// v = start value
	// c = end value
	// d = duration
//	static float easeIn (float t,float b , float c, float d) {
//		t/=d;
//		return c*t*t + b;
//	}
//	static float easeOut(float t,float b , float c, float d) {
//		t=t/d-1;
//		return c*(t*t + 1) + b;
//	}

//	static float easeInOut(float t, float b , float c, float d) {
//		t/=d/2;
//		if (t < 1) return c/2*t*t + b;
//		t-=2;
//		return c/2*(t*t + 2) + b;
//	}

	static float easeIn (float t,float b , float c, float d) {
		t/=d;
		return c*t*t + b;
	}
	static float easeOut(float t,float b , float c, float d) {
		t/=d;
		return -c *t*(t-2) + b;
	}

	static float easeInOut(float t,float b , float c, float d) {
		if ((t/=d/2) < 1) return ((c/2)*(t*t)) + b;
		return -c/2 * (((t-2)*(--t)) - 1) + b;
		/*
		 originally return -c/2 * (((t-2)*(--t)) - 1) + b;

		 I've had to swap (--t)*(t-2) due to diffence in behaviour in
		 pre-increment operators between java and c++, after hours
		 of joy
		 */

	}
};
