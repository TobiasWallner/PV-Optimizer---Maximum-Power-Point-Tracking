
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
	ExtremumSeekingController();

}

