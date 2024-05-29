/*
 * Author: Tobias Wallner
 * tobias.wallner1@gmx.net
 * */

// UART: 256000 Baud
extern "C"{

#include <DAVE.h>
#include <Interrupts.h>

}
#include <fix32.hpp>
#include <SineGenerator.hpp>
#include <Integrator.hpp>
#include <ExtremumSeekingController.hpp>
#include "print.hpp"

static inline uint32_t Vout_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Vout_filtered_handle);}
static inline uint32_t IL_filtered2_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_IL_filtered2_handle);}
static inline uint32_t Iin_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Iin_filtered_handle);}
static inline uint32_t IL_filtered1_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_IL_filtered1_handle);}
static inline uint32_t Vin_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Vin_filtered_handle);}

static inline fix32<16> output_voltage(){
	static const fix32<16> factor(0.025671838831018513f);
	return fix32<16>(Vout_filtered_raw()) * factor;
}

static inline fix32<16> Iin_current(){
	static const fix32<16> factor(0.00322265625f);
	static const fix32<16> offset(6.6f);
	return fix32<16>(Iin_filtered_raw()) * factor - offset;
}

static fix32<16> boost_loss(1L);

static inline fix32<16> output_current(){
	return Iin_current() * boost_loss;
}

static inline fix32<16> output_power(){
	return output_voltage() * output_current(); // boost_loss ... because parts of the current used for boost is put into ground instead of the output
}

// gain between 0 and 1 for boost and greater 1 for buck.
static inline void set_duty_cycles(fix32<16> gain){
	if(gain <= 0L){
		boost_loss = 1;
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  0L);
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Boost,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  0L);
	}else if(gain <= 1L){
		// buck mode
		boost_loss = 1;
		const uint32_t buck_pwm = static_cast<uint32_t>(gain * 10000L);
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  buck_pwm);
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_2,  buck_pwm*3/4+1); // triggers the ADC just before the power pwm switches (+1 so that the ADC also triggers at 0% duty cycle)
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Boost,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  0L);
	}else{
		// boost mode
		boost_loss = 1L/gain;
		const uint32_t boost_pwm = static_cast<uint32_t>((1L-boost_loss) * 10000L);
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  10000UL);
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Boost,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  boost_pwm);
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_2,  boost_pwm*3/4+1); // triggers the ADC just before the power pwm switches (+1 so that the ADC also triggers at 0% duty cycle)
	}
}

template<size_t N>
class MovingAverage{
	fix32<16> buffer[N]{};
	fix32<16> sum = 0;
	size_t index = 0;

public:

	inline MovingAverage(){for(fix32<16>& elem : buffer) elem = fix32<16>::reinterpret(0);}

	inline fix32<16> input(fix32<16> value){
		static const fix32<16> factor = 1 / fix32<16>(N);
		sum += (value - buffer[index]);
		buffer[index] = value;
		index = (index < (N-1)) ? index+1 : 0;
		return sum * factor;
	}

};

int main(void){
	DAVE_STATUS_t status;

	status = DAVE_Init();           /* Initialization of DAVE APPs  */

	if(status == DAVE_STATUS_FAILURE){
		/* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
		XMC_DEBUG("DAVE APPs initialization failed\n");

		while(1){

		}
	}

	// initialize the extremum seeking controller parameters
	const fix32<16> sample_time(0.01f); // 100Hz
	const fix32<16> driving_frequ(1.f * 2.f * 3.1415f);
	const fix32<16> driving_amplitude(0.05f);
	const fix32<16> integrator_gain(10);

	// driv
	SineGenerator<fix32<16>> sineGen(sample_time, driving_frequ, driving_amplitude);
	MovingAverage<100> movingAverage; // moving average has to be as large as 1 sine period

	Integrator<fix32<16>> integrator(sample_time, integrator_gain);

	/* Start PWM */
	PWM_CCU8_Start(&PWM_Buck);
	PWM_CCU8_Start(&PWM_Boost);
	TIMER_Start(&TIMER_Controller_Clock);

	//uint16_t update_counter = 0;
	fix32<16> voltage_filtered_x(0);
	fix32<16> voltage_filtered(0);

	fix32<16> current_filtered_x(0);
	fix32<16> current_filtered(0);

	size_t update_state = 0;

	while(1){
		/*filter here instead of in the interrupts because interrupts would reduce performance too much*/
		voltage_filtered_x = (voltage_filtered_x * 15 + output_voltage()*16)/16;
		voltage_filtered = voltage_filtered_x/16;

		current_filtered_x = (current_filtered_x * 15 + output_current()*16)/16;
		current_filtered = current_filtered_x/16;


		if(TIMER_GetInterruptStatus (&TIMER_Controller_Clock)){
			TIMER_ClearEvent (&TIMER_Controller_Clock);
			const fix32<16> U = voltage_filtered;
			const fix32<16> I = current_filtered;
			const fix32<16> P = U * I;

			const fix32<16> sine = sineGen.next();
			const fix32<16> correlation = movingAverage.input(P*sine);
			const fix32<16> integrator_output = integrator.input(correlation);

			const fix32<16> gain = sine + driving_amplitude + integrator_output;
			set_duty_cycles(gain);

			// output data over multiple iterations to distribute the UART load for less busy waiting
			switch(update_state){
				break; case 0:{cout << U << "V, "; ++update_state;}
				break; case 1:{cout << I << "A, "; ++update_state;}
				break; case 2:{cout << P << "W, "; ++update_state;}
				break; case 3:{cout << "gain: " << (gain*100) << "%"; ++update_state;}
				break; case 4:{cout << ", corr: " << correlation; ++update_state;}
				break; case 5:{cout << ", integ: " << integrator_output; ++update_state;}
				default: {cout << endl; update_state=0;}
			}
		}

	}
}

