#pragma once
/*
 * Board.hpp
 *
 *  Created on: 4 Jun 2024
 *      Author: Tobias Wallner
 *      tobias.wallner1@gmx.net
 */


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
		PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_2,  9900UL); // triggers the ADC measurement in the throughput time and not when the boost coil is being charged
	}
}
