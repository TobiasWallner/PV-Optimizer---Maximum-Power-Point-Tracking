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

static inline uint32_t Vout_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Vout_filtered_handle);}
static inline uint32_t IL_filtered2_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_IL_filtered2_handle);}
static inline uint32_t Iin_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Iin_filtered_handle);}
static inline uint32_t IL_filtered1_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_IL_filtered1_handle);}
static inline uint32_t Vin_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Vin_filtered_handle);}

static inline fix32<16> output_voltage(){
	static const fix32<16> factor(0.025671838831018513f);
	return fix32<16>(Vout_filtered_raw()) * factor;
}

static inline fix32<16> coil_current(){
	static const fix32<16> factor(0.00322265625f);
	static const fix32<16> offset(6.6f);
	return fix32<16>(Iin_filtered_raw()) * factor - offset;
}

static fix32<16> boost_loss(1L);

static inline fix32<16> output_current(){
	return coil_current() * boost_loss;
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
	const fix32<16> driving_frequ(0.1f * 2.f * 3.1415f);
	const fix32<16> driving_amplitude(0.05f); //
	const fix32<16> integrator_gain(1);

	ExtremumSeekingController<fix32<16>> esc(sample_time, driving_frequ, driving_amplitude, integrator_gain);

	/* Start PWM */
	PWM_CCU8_Start(&PWM_Buck);
	PWM_CCU8_Start(&PWM_Boost);
	TIMER_Start(&TIMER_Controller_Clock);

	uint16_t update_counter = 0;
	fix32<16> gain = 0;
	fix32<16> increment(0.0001f);
	//fix32<16> correlation_buffer[1000];
	//auto ptr = correlation_buffer;
	while(1){
		if(TIMER_GetInterruptStatus (&TIMER_Controller_Clock)){
			TIMER_ClearEvent (&TIMER_Controller_Clock);
			const auto U = output_voltage();
			const auto I = output_current();
			const auto P = U * I;
			gain = esc.input(P) + driving_amplitude*2;

			//*(ptr++) = P;

			set_duty_cycles(gain);

			++update_counter;
			if(update_counter>100){
				update_counter = 0;
				cout << U << "V, " << I << "A, " << P << "W, " << (gain*100) << "%" << endl;
			}
		}

	}
}

