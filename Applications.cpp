/*
 * Applications.cpp
 *
 *  Created on: 4 Jun 2024
 *      Author: Tobias Wallner
 *      tobias.wallner1@gmx.net
 */

#include <fix32.hpp>
#include <SineGenerator.hpp>
#include <Integrator.hpp>
#include <MovingAverage.hpp>

#include "print.hpp"
#include "board.hpp"

void BodeMeasurement(){
	// ! Expects the timer to trigger at 1 kHz !

	const fix32<16> sample_time(0.001f);
	const fix32<16> driving_frequ(1.f * 2.f * 3.1415f);
	const fix32<16> driving_amplitude(0.05f);


	const fix32<16> driving_frequencies[] = {
			1,
			2,
			3,
			4,
			5,
			6,
			7,
			8,
			9,
			10,
			20,
			30,
			40,
			50,
			60,
			70,
			80,
			90,
			100,
			200};

	const size_t number_of_frequencies = sizeof(driving_frequencies) / sizeof(driving_frequencies[0]);

	SineGenerator<fix32<16>> sineGen(sample_time, driving_frequ, driving_amplitude);

	fix32<16> sine_cross_corr(0);
	fix32<16> cos_cross_corr(0);

	fix32<16> sine_auto_corr(0);
	fix32<16> cos_auto_corr(0);

	size_t sample_index = 0;
	size_t freq_index = 0;

	cout << "frequency, sine_cross_corr, cos_cross_corr" << endl;

	uint32_t count = 0;

	while(true){
		++count;
		if(TIMER_GetInterruptStatus (&TIMER_Controller_Clock)){
			TIMER_ClearEvent (&TIMER_Controller_Clock);

			if(sample_index >= 8000){
				cout << driving_frequencies[freq_index] << ", " << sine_cross_corr << ", " << cos_cross_corr << endl;

				sample_index = 0;
				++freq_index;

				if(freq_index >= number_of_frequencies){
					set_duty_cycles(0);
					cout << "END" << endl;
					return;
				}

				sineGen.set_freq(driving_frequencies[freq_index] * 2.f * 3.1415f);

				sine_cross_corr = 0;
				cos_cross_corr = 0;
				sine_auto_corr = 0;
				cos_auto_corr = 0;
			}

			const fix32<16> U = output_voltage<fix32<16>>();
			const fix32<16> I = output_current<fix32<16>>();
			const fix32<16> P = U * I;

			sine_cross_corr += P * sineGen.sine();
			cos_cross_corr += P * sineGen.cosine();

			const fix32<16> gain = sineGen.sine() + fix32<16>(0.5);

			set_duty_cycles(gain);

			sineGen.next();
			++sample_index;
		}
	}

}

void ExtremumSeekingController(){
	// ! Expects the timer to trigger at 1 kHz !

	// initialize the extremum seeking controller parameters
	const fix32<16> sample_time(0.001f);
	const fix32<16> driving_frequ(10.f * 2.f * 3.1415f);
	const fix32<16> driving_amplitude(0.05f);
	const fix32<16> integrator_gain(1);
	const fix32<16> correlation_gain(1);
	const fix32<16> start_offset(0.1);

	// ESC components
	SineGenerator<fix32<16>> sineGen(sample_time, driving_frequ, driving_amplitude);
	MovingAverage<fix32<16>, 100> movingAverageCorrelation; // moving average has to be as large as 1 sine period
	Integrator<fix32<16>> integrator(sample_time, integrator_gain);

	size_t update_counter = 0;
	size_t update_state = 0;

	cout << "U, I, P, gain, correlation, integrator" << endl;


	while(true){
		if(TIMER_GetInterruptStatus (&TIMER_Controller_Clock)){
			TIMER_ClearEvent (&TIMER_Controller_Clock);
			const fix32<16> U = output_voltage();
			const fix32<16> I = output_current();
			const fix32<16> P = U * I;

			const fix32<16> sine = sineGen.next();
			movingAverageCorrelation.input(P*(sine*correlation_gain));
			const fix32<16> correlation = movingAverageCorrelation.sum(); // use the sum instead of the average to not loose information
			const fix32<16> integrator_output = integrator.input(correlation);

			if(integrator_output < 0) integrator.reset(); // prevent integrator from integrating into negative numbers for ever.

			const fix32<16> gain = sine + start_offset + integrator_output;
			set_duty_cycles(gain);

			if (update_counter > 100){
				update_counter = 0;
				switch(update_state){
					break; case 0:{cout << U << ", "; ++update_state;}
					break; case 1:{cout << I << ", "; ++update_state;}
					break; case 2:{cout << P << ", "; ++update_state;}
					break; case 3:{cout << gain << ", "; ++update_state;}
					break; case 4:{cout << correlation << ", "; ++update_state;}
					break; case 5:{cout << integrator_output << endl; update_state=0;}
					break; default: {cout << endl; update_state=0;}
				}
			}
			++update_counter;
			// output data over multiple iterations to distribute the UART load for less busy waiting

		}

	}
}





