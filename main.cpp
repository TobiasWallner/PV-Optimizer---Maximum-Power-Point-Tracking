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
#include <ExtremumSeekingController.hpp>
#include "print.hpp"

static inline fix32<16> output_voltage(){
	static const fix32<16> factor(0.025671838831018513f);
	return fix32<16>(adc_vout_raw_filtered) * factor;
}

static inline fix32<16> coil_current(){
	static const fix32<16> factor(0.00322265625f);
	static const fix32<16> offset(6.6f);
	return fix32<16>(adc_il_raw_filtered) * factor + offset;
}

static fix32<16> boost_loss(0L);

static inline fix32<16> output_current(){
	return coil_current() * boost_loss;
}

static inline fix32<16> output_power(){
	return output_voltage() * output_current(); // boost_loss ... because parts of the current used for boost is put into ground instead of the output
}

// gain between 0 and 1 for boost and greater 1 for buck.
static inline void set_duty_cycles(fix32<16> gain){
	if(gain <= 0L){
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  0L);
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Boost,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  0L);
	}else if(gain <= 1L){
		// buck mode
		const uint32_t buck_pwm = static_cast<uint32_t>(gain * 10000L);
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  buck_pwm);
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Boost,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  0L);
	}else{
		// boost mode
		boost_loss = 1L/gain;
		const uint32_t boost_pwm = static_cast<uint32_t>((1L-boost_loss) * 10000L);
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  10000UL);
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Boost,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  boost_pwm);
	}
}



int main(void){
	DAVE_STATUS_t status;

	status = DAVE_Init();           /* Initialization of DAVE APPs  */

	if(status == DAVE_STATUS_FAILURE){
		/* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
		XMC_DEBUG("DAVE APPs initialization failed\n");

		while(1){

		}
	}

	// initialize the ESC
	const fix32<16> sample_time(0.001f); // 1ms or 1kHz
	const fix32<16> driving_frequ(10.0f * 2.f * 3.1415f); // 10 Hz driving frequency
	const fix32<16> driving_amplitude(0.01f); // 1%
	const fix32<16> integrator_gain(1);

	ExtremumSeekingController<fix32<16>> esc(sample_time, driving_frequ, driving_amplitude, integrator_gain);

	/* Start PWM */
	PWM_CCU8_Start(&PWM_Buck);
	PWM_CCU8_Start(&PWM_Boost);
	TIMER_Start(&TIMER_Controller_Clock);

	cout << "U, I, P, gain" << endl;
	uint16_t update_counter = 0;
	while(1){
		if(TIMER_GetInterruptStatus (&TIMER_Controller_Clock)){
			TIMER_ClearEvent (&TIMER_Controller_Clock);
			const auto U = output_voltage();
			const auto I = output_current();
			const auto P = U * I;
			fix32<16> gain = fix32<16>(1); //esc.input(P);
			set_duty_cycles(gain);
			++update_counter;
			if(update_counter>250){
				cout << U << ", " << I << ", " << P << ", " << gain << endl;
				update_counter = 0;
			}
		}

	}
}

