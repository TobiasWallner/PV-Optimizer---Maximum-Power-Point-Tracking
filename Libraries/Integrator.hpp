/*
	Author: Tobias Wallner
	tobias.wallner1@gmx.net
*/

template<class T = float>
class Integrator{
private:
	T y = 0;
	T ki = 0;
	
public:

	 Integrator(T sample_time, T gain = T(1)) : ki(gain * sample_time){}
	
	 T input(T u){
		this->y += u * this->ki;
		return y;
	}
	
	friend T operator >> (T u, Integrator& I){return I.input(u);}
	
	 T get() const {return y;}
	
	 void reset() {
		this->y = 0;
	}

};
