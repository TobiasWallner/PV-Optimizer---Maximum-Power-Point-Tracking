PV-Optimizer
------------

A PV-Optimizer Algorithm using modulated sine waves and their correlation to determine the gradient. 

File Structure
--------------

* **main.cpp** - the main execution file, initialises the chip and starts the application program.
 
* **Application.hpp/cpp** - contains the application programs: "Extremum Seeking Controller" & "Bode Diagram"

* **board.cpp/hpp** - contains abstractions for interacting with the board PWM and ADC Measurements

* **print.cpp/hpp** - provides printing and streaming functionality.

* **Libraries/**
	* **fix32.hpp** - A templated 32-bit wide fixpoint with a user selectable number of fractional bits.
	* **Integrator.hpp** - A classical integrator from signal and systems theory
	* **MovingAverage.hpp** - A fast moving average for types of integral (~integer) precision.
	* **SineGenerator.hpp** - A sine generator based on the wave function.
	
