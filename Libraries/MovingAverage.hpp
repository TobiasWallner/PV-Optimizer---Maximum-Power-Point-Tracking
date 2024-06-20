#pragma once
/*
 * MovingAverage.hpp
 *
 *  Created on: 4 Jun 2024
 *      Author: Tobias Wallner
 */

template<class T, size_t N>
class MovingAverage{
	T _buffer[N]{};
	T _sum = T(0);
	size_t _index = 0;

public:

	inline MovingAverage(){for(T& elem : this->_buffer) elem = T(0);}

	inline T input(T value){
		this->_sum += (value - this->_buffer[this->_index]);
		this->_buffer[this->_index] = value;
		this->_index = (this->_index < (N-1)) ? this->_index+1 : 0;
		T result = this->_sum / N;
		return result;
	}

	inline T sum() const {return this->_sum;}
	inline T average() const {return this->_sum/N;}

	template<class Stream>
	friend Stream& operator << (Stream& stream, const MovingAverage& ma){
		stream << "sum: " << ma.sum() << "\n";
		stream << "values: ";
		for (const auto& elem : ma._buffer){
			stream << elem << ", ";
		}
	}


};
