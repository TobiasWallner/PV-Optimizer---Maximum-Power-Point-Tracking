/*
 * Interrupts.c
 *
 *  Created on: 21 May 2024
 *      Author: SirTobi
 */

/*
	!!! This file is depreciated !!!
	It seemend the ADC interrupts came too often stalling the normal operation of the CPU
*/

#include <DAVE.h>

uint32_t adc_vout_raw_filtered = 0;
 void ISR_Voltage_Measurement(){
	 static uint32_t x = 0;
	 //
	 // G = 3.24 / (100 + 3.24) // Spannungsteiler Gain
	 // adc_bit_range = 1 << 12
	 // adc_voltage_range = 3.3
	 //
	 // adc_vout_raw = G * adc_bit_range / adc_voltage_range * Vout
	 // --> adc_vout_raw = 38.9531893910042 * Vout
	 // --> Vout = 0.025671838831018513 * adc_vout_raw
	 //

	const uint32_t adc_vout_raw = 0;//(uint32_t)(ADC_MEASUREMENT_ADV_GetResult(&ADC_Voltage_Voltage_handle));
	const uint32_t n = 5; // filter over 2^n samples
	const uint32_t u = adc_vout_raw << n;
	x = (x*((1<<n) - 1) + u) >> n;
	adc_vout_raw_filtered = x >> n;

}

uint32_t adc_il_raw_filtered = 0;

void ISR_Current_Measurement(){
	static uint32_t x = 0;
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

	const uint32_t adc_il_raw = 0;//(uint32_t)(ADC_MEASUREMENT_ADV_GetResult(&ADC_Current_Current_handle));
	const uint32_t n = 5; // filter over 2^n samples
	const uint32_t u = adc_il_raw << n;
	x = (x*((1<<n) - 1) + u) >> n;
	adc_il_raw_filtered = x >> n;
}
