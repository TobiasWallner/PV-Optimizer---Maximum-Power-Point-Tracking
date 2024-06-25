#pragma once
/*
 * Board.hpp
 *
 *  Created on: 4 Jun 2024
 *      Author: Tobias Wallner
 *      tobias.wallner1@gmx.net
 */

/*
	This header file accts like a basic BIOS for the PWM and ADC Measurements of the board. 
*/

// functions to get the raw measurements from the ADC. 
inline uint32_t Vout_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Vout_filtered_handle);}
inline uint32_t IL_filtered2_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_IL_filtered2_handle);}
inline uint32_t Iin_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Iin_filtered_handle);}
inline uint32_t IL_filtered1_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_IL_filtered1_handle);}
inline uint32_t Vin_filtered_raw() {return ADC_MEASUREMENT_ADV_GetResult(&ADC_Vin_filtered_handle);}

// Conversion of adc_raw to volt
// -----------------------------
// G = 3.24 / (100 + 3.24) // Spannungsteiler Gain
// adc_bit_range = 1 << 12
// adc_voltage_range = 3.3
//
// adc_vout_raw = G * adc_bit_range / adc_voltage_range * Vout
// --> adc_vout_raw = 38.9531893910042 * Vout
// --> Vout = 0.025671838831018513 * adc_vout_raw
//

// Conversion of adc_raw to ampere
// -------------------------------
// R_m = 5 * 10**(-3) // Messwiderstand
// G = 50 // Gain
// offset = 1.65
// adc_bit_range = 1 << 12
// adc_voltage_range = 3.3
//
// adc_iout_raw = (I_L * R_m * G + offset) * adc_bit_range / adc_voltage_range
//
// --> adc_iout_raw = I * 310,30... + 2048
// --> I_L = adc_iout_raw * 0.00322265625 - 6.6
// --> I_out = I_L * Duty_Boost

// function that converts a raw_adc value into a voltage unit. Optionally the output type can be specified. Default is fix32<16>
template<class T = fix32<16>>
inline T convert_raw_adc_to_volt(uint32_t raw_adc){
	const T factor(0.025671838831018513f);
	return T(raw_adc) * factor;
}

// converts the raw adc measurement to an ampere value
template<class T = fix32<16>>
inline T convert_raw_adc_to_ampere(uint32_t raw_adc){
	const T factor(0.00322265625f);
	const T offset(6.6f);
	return T(raw_adc) * factor - offset;
}

// returns the output voltage measurement in volts
template<class T = fix32<16>>
inline T output_voltage(){
	const T factor(0.025671838831018513f);
	return T(Vout_filtered_raw()) * factor;
}

// returns the input current of the PV-Module in ampere
template<class T = fix32<16>>
inline T Iin_current(){
	const T factor(0.00322265625f);
	const T offset(6.6f);
	return T(Iin_filtered_raw()) * factor - offset;
}

// variable that stores the boost loss coefficient.
static fix32<16> boost_loss(1L);

// returns the estimated output current based on the input current and the current pwm-dutycycle / boost_loss
template<class T = fix32<16>>
inline T output_current(){
	return Iin_current() * boost_loss;
}

// returns the estimated output power of the optimizer board
template<class T = fix32<16>>
inline T output_power(){
	return output_voltage() * output_current(); // boost_loss ... because parts of the current used for boost is put into ground instead of the output
}

// enum to distinguish between boost and buck mode
enum class DutyCycleType{buck, boost};

// set the duty cycle for a given mode (buck or boost)
// the duty_cycle input is in range of [0, 10000UL] that corresponds to [0%, 100%] duty cycle.
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

// sets the duty cycle based on the given gain. 
// The gain can be seen as the input output voltage amplification of the board: gain = V_out / V_in
// Thus gain values in [0, 1] will be mapped to the buck mode and [1, inf] will be mapped to the 
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
		setDutyCycle(boost_pwm, DutyCycleType::boost);
	}
}
