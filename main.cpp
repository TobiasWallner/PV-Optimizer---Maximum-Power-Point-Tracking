
/*
 * Author: Tobias Wallner
 * tobias.wallner1@gmx.net
 * */

// UART: 256000 Baud
extern "C"{
	#include <DAVE.h>
}

#include <fix32.hpp>
#include <SineGenerator.hpp>
#include <Integrator.hpp>
#include <MovingAverage.hpp>

#include "print.hpp"
#include "board.hpp"
#include "Applications.hpp"



int main(void){
	DAVE_STATUS_t status;

	status = DAVE_Init();           /* Initialization of DAVE APPs  */

	if(status == DAVE_STATUS_FAILURE){
		/* Placeholder for error handler code. The while loop below can be replaced with an user error handler. */
		XMC_DEBUG("DAVE APPs initialization failed\n");

		while(1){

		}
	}

	/* Start PWM */
	PWM_CCU8_Start(&PWM_Buck);
	PWM_CCU8_Start(&PWM_Boost);
	TIMER_Start(&TIMER_Controller_Clock);

	//while(true) BodeMeasurement();
	//ExtremumSeekingController();

	uint32_t duty_cycle = 60;
	auto DutyType = DutyCycleType::boost;
	setDutyCycle(duty_cycle * 100, DutyType);

	uint32_t update_counter = 0;

	MovingAverage<fix32<16>, 50> ma_Vout_filtered_raw;
	MovingAverage<fix32<16>, 50> ma_Vin_filtered_raw;
	MovingAverage<fix32<16>, 50> ma_Iin_filtered_raw;
	MovingAverage<fix32<16>, 50> ma_IL_filtered1_raw;
	MovingAverage<fix32<16>, 50> ma_IL_filtered2_raw;


	while(true){
		ma_Vout_filtered_raw.input(convert_raw_adc_to_volt(Vout_filtered_raw()));
		ma_Vin_filtered_raw.input(convert_raw_adc_to_volt(Vin_filtered_raw()));
		ma_Iin_filtered_raw.input(convert_raw_adc_to_ampere(Iin_filtered_raw()));
		ma_IL_filtered1_raw.input(convert_raw_adc_to_ampere(IL_filtered1_raw()));
		ma_IL_filtered2_raw.input(convert_raw_adc_to_ampere(IL_filtered2_raw()));


		if(TIMER_GetInterruptStatus (&TIMER_Controller_Clock)){
			TIMER_ClearEvent (&TIMER_Controller_Clock);

			if (update_counter > 500){
				update_counter = 0;

				cout << duty_cycle << ", ";
				cout << ((DutyType == DutyCycleType::buck) ? "buck" : "boost") << ", ";
				cout << ma_Vout_filtered_raw.average() << ", ";
				cout << ma_Vin_filtered_raw.average() << ", ";

				cout << ma_Iin_filtered_raw.average() << ", ";
				cout << ma_IL_filtered1_raw.average() << ", ";
				cout << ma_IL_filtered2_raw.average() << ", ";
				cout << endl;


			}
			++update_counter;

		}

	}
}

