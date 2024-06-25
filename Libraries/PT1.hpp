#pragma once

/*
 * Author: Tobias Wallner
 * tobias.wallner1@gmx.net
 * */

/*
	PT1
		A PT1 (Proportional time dely of order 1) 
		element that uses euler approximation instead of exact discretisation
*/

template<class T = float>
class PT1{
private:
	T y = 0;
	T a = 0;
	
public:

	// constructs the PT1 element based on the sample time and characteristic frequency
	 PT1(const T& sample_time, const T& w_n) : a(w_n * sample_time){}
		
	// provide a new input to the PT1 element updating its value	
	 T input(const T& u){
		this->y += (u - this->y) * this->a;
		return y;
	}
	
	// provide a forward streaming operator to concattenate multiple filters
	friend T operator >> (const T& u, PT1& pt1){return pt1.input(u);}
	
	// get the current value
	 T get() const {return y;}
	
	// reset internal states
	 void reset() {
		this->a = 0;
		this->y = 0;
	}
	
};
