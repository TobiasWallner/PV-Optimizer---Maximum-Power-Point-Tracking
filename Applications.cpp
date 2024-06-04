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

	const fix32<16> driving_frequencies[5] = {
			1.f * 2.f * 3.1415f,
			2.f * 2.f * 3.1415f,
			3.f * 2.f * 3.1415f,
			4.f * 2.f * 3.1415f,
			5.f * 2.f * 3.1415f};

	SineGenerator<fix32<16>> sineGen(sample_time, driving_frequ, driving_amplitude);

	fix32<16> sine_corr(0);
	fix32<16> cos_corr(0);

	int sample_index = 0;
	int freq_index = 0;

	cout << "frequency, sine_corr, cos_corr" << endl;

	uint32_t count = 0;

	while(true){
		++count;
		if(TIMER_GetInterruptStatus (&TIMER_Controller_Clock)){
			TIMER_ClearEvent (&TIMER_Controller_Clock);

			if(sample_index >= 1000){
				cout << driving_frequencies[freq_index] << ", " << sine_corr << ", " << cos_corr << endl;

				sample_index = 0;
				++freq_index;

				if(freq_index>=5){
					set_duty_cycles(0);
					cout << "END" << endl;
					while(true){/*trap*/}
				}

				sineGen.set_freq(driving_frequencies[freq_index]);

				sine_corr = 0;
				cos_corr = 0;
			}

			const fix32<16> U = output_voltage();
			const fix32<16> I = output_current();
			const fix32<16> P = U * I;

			sine_corr += P * sineGen.sine();
			cos_corr += P * sineGen.cosine();
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
	const fix32<16> driving_frequ(1.f * 2.f * 3.1415f);
	const fix32<16> driving_amplitude(0.05f);
	const fix32<16> integrator_gain(10);

	// ESC components
	SineGenerator<fix32<16>> sineGen(sample_time, driving_frequ, driving_amplitude);
	MovingAverage<100> movingAverageCorrelation; // moving average has to be as large as 1 sine period
	Integrator<fix32<16>> integrator(sample_time, integrator_gain);

	// measurement filters
	MovingAverage<16> movingAverageVoltage;
	MovingAverage<16> movingAverageCurrent;

	size_t update_state = 0;

	while(true){
		/*filter here instead of in the interrupts because interrupts would reduce performance too much*/
		const fix32<16> voltage_filtered = movingAverageVoltage.input(output_voltage());
		const fix32<16> current_filtered = movingAverageCurrent.input(output_current());

		if(TIMER_GetInterruptStatus (&TIMER_Controller_Clock)){
			TIMER_ClearEvent (&TIMER_Controller_Clock);
			const fix32<16> U = voltage_filtered;
			const fix32<16> I = current_filtered;
			const fix32<16> P = U * I;

			const fix32<16> sine = sineGen.next();
			const fix32<16> correlation = movingAverageCorrelation.input(P*sine);
			const fix32<16> integrator_output = integrator.input(correlation);

			const fix32<16> gain = sine + driving_amplitude + integrator_output;
			set_duty_cycles(gain);

			// output data over multiple iterations to distribute the UART load for less busy waiting
			switch(update_state){
				break; case 0:{cout << U << "V, "; ++update_state;}
				break; case 1:{cout << I << "A, "; ++update_state;}
				break; case 2:{cout << P << "W, "; ++update_state;}
				break; case 3:{cout << "gain: " << (gain*100) << "%"; ++update_state;}
				break; case 4:{cout << ", corr: " << correlation; ++update_state;}
				break; case 5:{cout << ", integ: " << integrator_output; ++update_state;}
				default: {cout << endl; update_state=0;}
			}
		}

	}
}





