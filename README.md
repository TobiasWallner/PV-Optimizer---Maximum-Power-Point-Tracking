PV-Optimizer
------------

A PV-Optimizer Algorithm using modulated sine waves and their correlation to determine the gradient. 

Outputs measurements periodically over the USART at 256000 Baud like in a csv file:
first line is the header:
following line are the corresponding measurements seperated by ',' as column separators 

Example output:
```
U, I, P, gain, correlation, integrator
1.976, 0.1676, 0.6826, 0.2488, 0.09332, 0.1619
4.055, 0.1225, 0.7073, 0.3449, -0.6204, 0.4696
10.445, 1.030, 7.931, 0.9803, -2.254, 0.6792
```

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
	
Branch Structure
----------------
The following branches are snapshots that compile, work and the algorithm seems to be able to find the maxima.
* ESC_v1.0
* ESC_v1.1
* ESC_v1.2
* main - newest: lowest driving amplitude with 1uF caps for Vout and Iin

Building
--------
Build with at least g++ version 4.5 and C++14. In the DAVE IDE this can be selected in Project>Properties>C/C++Build>Settings>ARM-GCC C Compiler>Miscellaneous>Language Standard