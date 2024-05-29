#pragma once

/*
 * Author: Tobias Wallner
 * tobias.wallner1@gmx.net
 * */

template<class T = float>
class PT1{
private:
	T y = 0;
	T a = 0;
	
public:

	 PT1(const T& sample_time, const T& w_n) : a(w_n * sample_time){}
		
	 T input(const T& u){
		this->y += (u - this->y) * this->a;
		return y;
	}
	
	friend T operator >> (const T& u, PT1& pt1){return pt1.input(u);}
	
	 T get() const {return y;}
	
	 void reset() {
		this->a = 0;
		this->y = 0;
	}
	
};
