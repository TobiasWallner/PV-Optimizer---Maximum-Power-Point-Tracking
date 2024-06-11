#pragma once
/*
 * Board.hpp
 *
 *  Created on: 4 Jun 2024
 *      Author: Tobias Wallner
 *      tobias.wallner1@gmx.net
 */


inline uint32_t Vout_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Vout_filtered_handle);}
inline uint32_t IL_filtered2_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_IL_filtered2_handle);}
inline uint32_t Iin_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Iin_filtered_handle);}
inline uint32_t IL_filtered1_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_IL_filtered1_handle);}
inline uint32_t Vin_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Vin_filtered_handle);}

template<class T = fix32<16>>
inline T convert_raw_adc_to_volt(uint32_t raw_adc){
	const T factor(0.025671838831018513f);
	return T(raw_adc) * factor;
}

template<class T = fix32<16>>
inline T convert_raw_adc_to_ampere(uint32_t raw_adc){
	const T factor(0.00322265625f);
	const T offset(6.6f);
	return T(raw_adc) * factor - offset;
}

template<class T = fix32<16>>
inline T output_voltage(){
	const T factor(0.025671838831018513f);
	return T(Vout_filtered_raw()) * factor;
}

template<class T = fix32<16>>
inline T Iin_current(){
	const T factor(0.00322265625f);
	const T offset(6.6f);
	return T(Iin_filtered_raw()) * factor - offset;
}

static fix32<16> boost_loss(1L);

template<class T = fix32<16>>
inline T output_current(){
	return Iin_current() * boost_loss;
}

template<class T = fix32<16>>
inline T output_power(){
	return output_voltage() * output_current(); // boost_loss ... because parts of the current used for boost is put into ground instead of the output
}

enum class DutyCycleType{buck, boost};

inline void setDutyCycle(uint32_t duty_cycle, DutyCycleType type){
	switch(type){
		case DutyCycleType::buck : {
			PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  duty_cycle);
			PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_2,  duty_cycle*3/4+1); // triggers the ADC just before the power pwm switches (+1 so that the ADC also triggers at 0% duty cycle)
			PWM_CCU8_SetDutyCycleSymmetric(&PWM_Boost,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  0L);
		} break;
		case DutyCycleType::boost : {
			PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  10000UL);
			PWM_CCU8_SetDutyCycleSymmetric(&PWM_Boost,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  duty_cycle);
			PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_2,  9900UL); // triggers the ADC measurement in the throughput time and not when the boost coil is being charged
		} break;
		default: {
			PWM_CCU8_SetDutyCycleSymmetric(&PWM_Buck,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  0L);
			PWM_CCU8_SetDutyCycleSymmetric(&PWM_Boost,  XMC_CCU8_SLICE_COMPARE_CHANNEL_1,  0L);
		}
	}
}

// gain between 0 and 1 for boost and greater 1 for buck.
template<class T = fix32<16>>
inline void set_duty_cycles(T gain){
	if(gain <= 0L){
		boost_loss = 1;
		setDutyCycle(0, DutyCycleType::buck);
	}else if(gain <= 1L){
		// buck mode
		boost_loss = 1;
		const uint32_t buck_pwm = static_cast<uint32_t>(gain * 10000L);
		setDutyCycle(buck_pwm, DutyCycleType::buck);
	}else{
		// boost mode
		boost_loss = 1L/gain;
		const uint32_t boost_pwm = static_cast<uint32_t>((1L-boost_loss) * 10000L);
		setDutyCycle(boost_pwm, DutyCycleType::buck);
	}
}
