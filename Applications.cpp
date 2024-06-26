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

	// define parameters for the sine wave
	const fix32<16> sample_time(0.001f);
	const fix32<16> driving_frequ(1.f * 2.f * 3.1415f);
	const fix32<16> driving_amplitude(0.05f);

	// define the frequencies at which to sample the bode diagram
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

	// instanciate the sine generator
	SineGenerator<fix32<16>> sineGen(sample_time, driving_frequ, driving_amplitude);

	// fixpoint variables that measure store the correlation
	fix32<16> sine_cross_corr(0);
	fix32<16> cos_cross_corr(0);
	fix32<16> sine_auto_corr(0);
	fix32<16> cos_auto_corr(0);

	size_t sample_index = 0;
	size_t freq_index = 0;

	// output csv header
	cout << "frequency, sine_cross_corr, cos_cross_corr" << endl;

	uint32_t count = 0;

	while(true){
		++count;
		if(TIMER_GetInterruptStatus (&TIMER_Controller_Clock)){
			TIMER_ClearEvent (&TIMER_Controller_Clock);
			
			// sample over 8000 steps, then output the results
			if(sample_index >= 8000){
				
				// output results
				cout << driving_frequencies[freq_index] << ", " << sine_cross_corr << ", " << cos_cross_corr << endl;

				// load next frequency
				sample_index = 0;
				++freq_index;
				
				// end condition
				if(freq_index >= number_of_frequencies){
					set_duty_cycles(0);
					cout << "END" << endl;
					return;
				}

				// set new frequency
				sineGen.set_freq(driving_frequencies[freq_index] * 2.f * 3.1415f);

				// initialise next iteration
				sine_cross_corr = 0;
				cos_cross_corr = 0;
				sine_auto_corr = 0;
				cos_auto_corr = 0;
			}
			
			// get measurements
			const fix32<16> U = output_voltage<fix32<16>>();
			const fix32<16> I = output_current<fix32<16>>();
			const fix32<16> P = U * I;

			// calculate correlation
			sine_cross_corr += P * sineGen.sine();
			cos_cross_corr += P * sineGen.cosine();

			// output new sine value
			const fix32<16> gain = sineGen.sine() + fix32<16>(0.5);
			set_duty_cycles(gain);

			// increment for next iteration
			sineGen.next();
			++sample_index;
		}
	}

}

void ExtremumSeekingController(){
	// ! Expects the timer to trigger at 1 kHz !

	/*
		This algorithm can get stuck in local maxima that are smaller than the driving amplitude.
		One way of avoiding this is increasing the driving amplitude. Note that when increasing the
		driging amplitude, the gain of the controller rises too, so you might want to decrease the
		correlation gain or the integrator gain.
		
		Also note that when the gain of this controller is chosen to be too large and the slope of the
		integrator output becomes close to the slope of the 20Hz driving signal, the controller may become instable.
		If the Controller becomes instable the correlation will turn strongly negative which effectively
		runs the bord down to a zero gain or zero duty-cycle. This will act as its fail-safe point from where
		it will then try to restart its operation. 
	*/
	
	/*
		Outputs measurements periodically over the USART at 256000 Baud like in a csv file:
		first line is the header:
			"U, I, P, gain, correlation, integrator\n"
		
		following line are the corresponding measurements seperated by ',' as column separators 
		and '\n' as newline separators
	*/

	/*
		This version uses the lowest driving amplitude that could only be achieved after replacing the
		Capacitors of Vout and Iin from 1nF to 1uF.

		The lowest driving amplitude for 1nF capacitors was 0.05 to not be stuck at local maxima in the
		regoin of 0-20% Duty-Cycle
	*/

	// initialize the extremum seeking controller parameters
	const fix32<16> sample_time(0.001f); // The period of the TIMER_Controller_Clock
	const fix32<16> driving_frequ(20.f * 2.f * 3.1415f);
	const fix32<16> driving_amplitude(0.02f);
	const fix32<16> integrator_gain(1); // change the correlation_gain instead to have more numeric accuracy
	const fix32<16> correlation_gain(4);
	const fix32<16> start_offset(driving_amplitude);

	// ESC components
	SineGenerator<fix32<16>> sineGen(sample_time, driving_frequ, driving_amplitude);
	MovingAverage<fix32<16>, 50> movingAverageCorrelation; // moving average has to be as large as 1 sine period
	Integrator<fix32<16>> integrator(sample_time, integrator_gain);

	size_t update_counter = 0;

	cout << "U, I, P, gain, correlation, integrator" << endl;


	while(true){
		if(TIMER_GetInterruptStatus (&TIMER_Controller_Clock)){
			TIMER_ClearEvent (&TIMER_Controller_Clock);
			
			// load measurement results
			const fix32<16> U = output_voltage();
			const fix32<16> I = output_current();
			const fix32<16> P = U * I;

			// perform the ESC algorithm
			const fix32<16> sine = sineGen.next();
			movingAverageCorrelation.input(P*(sine*correlation_gain));
			// use the sum instead of the average to not loose information
			const fix32<16> correlation = movingAverageCorrelation.sum(); 
			const fix32<16> integrator_output = integrator.input(correlation);

			// when no input power / output load is connected the correlation may get negative and the integrator reduces count.
			// prevent iterator to count to negative infinity. Resetting the integrator makes the algorithm more available.
			if(integrator_output < fix32<16>(-0.1f)) integrator.reset(); 

			// output new gain. 
			const fix32<16> gain = sine + start_offset + integrator_output;
			set_duty_cycles(gain);

			// print results with large intervalls to not busy wait for buffers to flush
			if (update_counter > 100){
				update_counter = 0;
				cout << U << ", ";
				cout << I << ", ";
				cout << P << ", ";
				cout << gain << ", ";
				cout << correlation << ", ";
				cout << integrator_output << "\n";
			}
			++update_counter;
		}

	}
}





