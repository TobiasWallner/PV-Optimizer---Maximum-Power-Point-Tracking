#pragma once
/*
 * MovingAverage.hpp
 *
 *  Created on: 4 Jun 2024
 *      Author: Tobias Wallner
 */

template<size_t N>
class MovingAverage{
	fix32<16> buffer[N]{};
	fix32<16> sum = 0;
	size_t index = 0;

public:

	inline MovingAverage(){for(fix32<16>& elem : buffer) elem = fix32<16>::reinterpret(0);}

	inline fix32<16> input(fix32<16> value){
		static const fix32<16> factor = 1 / fix32<16>(N);
		sum += (value - buffer[index]);
		buffer[index] = value;
		index = (index < (N-1)) ? index+1 : 0;
		return sum * factor;
	}

};
