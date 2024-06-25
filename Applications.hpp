#pragma once
/*
 * Applications.hpp
 *
 *  Created on: 4 Jun 2024
 *      Author: Tobias Wallner
 *      tobias.wallner1@gmx.net
 */

/*
	BodeMeasurement
		applies a sine wave to the pwm duty cycle and measures the correlation with the sine and cosine
*/
void BodeMeasurement();
/*
	ExtremumSeekingController
	
	Applies modulates a sine wave over the controll signal which is then used to derive the 
	gradient. Then the controll signal will rise on a positive gradiendt and fall on a negative gradient
	
              _______          _______         ______________            _______
    u ------> |     |    r    | low  |    g    |             |    k      |     |
              |  *  | ------> | pass | ------> | integrator  | --------> |     |
       +----> |_____|         |______|         |_____________|           |  +  | ------> y
       |                                                            +--> |     |
       |     delayed_sine         _________    sine                 |    |_____|
       +------------------------- | phase | <----------------+------+
                                  | delay |    cosine        |
                                  |_______| <-----------+    |
                                                        |    |
                                                        |    |
                                                    _____________
                                                    | Sine      |
                                                    | Generator |
                                                    |___________|
*/
void ExtremumSeekingController();
