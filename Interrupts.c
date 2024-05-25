/*
 * Interrupts.c
 *
 *  Created on: 21 May 2024
 *      Author: SirTobi
 */

#include <DAVE.h>

uint32_t adc_vout_raw = 0;
 void ISR_Voltage_Measurement(){
	adc_vout_raw = (uint32_t)(ADC_MEASUREMENT_ADV_GetResult(&ADC_Voltage_Voltage_handle));

}

uint32_t adc_iout_raw = 0;
void ISR_Current_Measurement(){
	adc_iout_raw = (uint32_t)(ADC_MEASUREMENT_ADV_GetResult(&ADC_Current_Current_handle));
}
