#pragma once
/*
	Author: Tobias Wallner
	tobias.wallner1@gmx.net
*/

/*
	Integrator:
		Class that provides the classical implementation of an integrator from signals and systems theory
*/

template<class T = float>
class Integrator{
private:
	T y = 0;
	T ki = 0;
	
public:

	// contruct from sample time and gain. 
	Integrator(T sample_time, T gain = T(1)) : ki(gain * sample_time){}
	
	// enter new input
	T input(T u){
		this->y += u * this->ki;
		return y;
	}
	
	// forward new value as input
	friend T operator >> (T u, Integrator& I){return I.input(u);}
	
	// returns the current output value
	T get() const {return y;}
	
	// resets the internal state
	void reset() {
		this->y = 0;
	}

};
