/*
	Author: Tobias Wallner
	tobias.wallner1@gmx.net	
*/

#include <PT1.hpp>
#include <Integrator.hpp>
#include <SineGenerator.hpp>

/*
              _______          _______         ______________            _______
    u ------> |     |    r    | low  |    g    |             |    k      |     |
              |  *  | ------> | pass | ------> | integrator  | --------> |     |
       +----> |_____|         |______|         |_____________|           |  +  | ------> y
       |                                                            +--> |     |
       |     delayed_sine         _________    sine                 |    |_____|
       +------------------------- | phase | <----------------+------+
                                  | delay |    cosine        |
                                  |_______| <-----------+    |
                                                        |    |
                                                        |    |
                                                    _____________
                                                    | Sine      |
                                                    | Generator |
                                                    |___________|
*/

template<class T = float>
class ExtremumSeekingController{
private:
	SineGenerator<T> sine_generator;
	PT1<T> lowpass;
	Integrator<T> integrator;
	T y;
	
public:

	/*
		note: the phase delay in degree is only valid in the range of [0, 90], otherwase the behaviour is undefined
	*/
	 ExtremumSeekingController(T sample_time_s, T driving_frequ_rad_s, T driving_amplitude = T(1), T integrator_gain = T(1))
		: sine_generator(sample_time_s, driving_frequ_rad_s, driving_amplitude)
		, lowpass(sample_time_s, driving_frequ_rad_s/T(10))
		, integrator(sample_time_s, integrator_gain)
		, y(0L){}
		
	 T get() const {
		return this->y;
	}
	
	 T input(T u){
		const auto sine = sine_generator.next();
		const auto r = u * sine;
		const auto g = lowpass.input(r);
		const auto k = integrator.input(g);
		this->y = sine + k;
		return this->y;
	}
	
	 void reset(){
		sine_generator.reset();
		lowpass.reset();
		integrator.reset();
	}
	
	friend T operator >> (T u, ExtremumSeekingController& esc){return esc.input(u);}
	
};
