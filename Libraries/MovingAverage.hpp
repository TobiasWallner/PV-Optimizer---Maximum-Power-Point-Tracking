#pragma once
/*
 * MovingAverage.hpp
 *
 *  Created on: 4 Jun 2024
 *      Author: Tobias Wallner
 */

/*
	MovingAverage
		A Moving Average filter that is optimised for calculation speed. 
		Is only precise if the input value type is of integral precision. 
		
		Calculates the sum/average not by summing the contained values but by
		keeping track of the ingoing and outgoing values. Contained values
		are stored in a single port zircular buffer.
*/

template<class T, size_t N>
class MovingAverage{
	T _buffer[N]{};
	T _sum = T(0);
	size_t _index = 0;

public:

	// construct an empty MovingAverage filter
	inline MovingAverage(){for(T& elem : this->_buffer) elem = T(0);}

	// apply an input to the moving average updating its value
	inline T input(T value){
		this->_sum += (value - this->_buffer[this->_index]);
		this->_buffer[this->_index] = value;
		this->_index = (this->_index < (N-1)) ? this->_index+1 : 0;
		T result = this->_sum / N;
		return result;
	}

	// returns the sum of the contained values
	inline T sum() const {return this->_sum;}
	
	// returns the average of the contained values
	inline T average() const {return this->_sum/N;}

	// prints the content of this object to an output stream 
	template<class Stream>
	friend Stream& operator << (Stream& stream, const MovingAverage& ma){
		stream << "sum: " << ma.sum() << "\n";
		stream << "values: ";
		for (const auto& elem : ma._buffer){
			stream << elem << ", ";
		}
	}


};
