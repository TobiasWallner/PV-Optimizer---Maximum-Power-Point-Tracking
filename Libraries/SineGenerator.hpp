#pragma once
/*
 * Author: Tobias Wallner
 * 	tobias.wallner1@gmx.net
 * */
 
 /*
	An efficient sine generator derived from the differential equations of a wave function and
	discretised using eulers method instead of exact discretisation.
 */

template<class T = float>
class SineGenerator{
private:
	T sin_w_t;
	T cos_w_t;
	T w;
	T dt;
	T amplitude;
	
public:
	
	// constructs a sine generator based on the sample time, output frequency, amplitude and 
	SineGenerator(T sample_time_s, T freq_rad_s, T amplitude = 1L)
		: sin_w_t(T(0L))
		, cos_w_t(T(1L))
		, w(freq_rad_s)
		, dt(sample_time_s)
		, amplitude(amplitude){}
		
	 void set_freq(T freq_rad_s){
		this->w = freq_rad_s;
	}
	
	// set the sample time
	 void set_sample_time(T sample_time_s){
		this->dt = sample_time_s;
	}
	
	// reset internal states
	 void reset(){
		this->sin_w_t = T(0);
		this->cos_w_t = T(1);
	}
	
	// get the current value of the sine without advancing it
	 T sine() const {return this->sin_w_t * this->amplitude;}
	 
	// get the current value of the cosine without advancing it
	 T cosine() const {return this->cos_w_t * this->amplitude;}
	
	// returns the current sine
	 T get() const {return this->sine();}
	
	// adds a slider in the range of [-1 ... 0 ... +1]
	// where 	-1 	returns the current -cosine
	//			0	returns the current sine
	//			+1	returns the current cosine
	 T get(T phi_per_pi_quarter) const {
		const auto abs_t = (phi_per_pi_quarter >= 0L) ? phi_per_pi_quarter : -phi_per_pi_quarter;
		const auto sqrt_2 = T(1.414213562);
		const auto one = T(1L);
		const auto v = (abs_t * abs_t + abs_t) * sqrt_2 + one;
		return ((one - abs_t) * this->sin_w_t + abs_t * this->cos_w_t) * v * this->amplitude;
	}
	
	// advance the sine generator and returns the current value of the sine. 
	 T next() {
		const T result = this->sine();
		this->cos_w_t -= (this->sin_w_t * this->w) * this->dt;
		this->sin_w_t += (this->cos_w_t * this->w) * this->dt;
		return result;
	}
};
