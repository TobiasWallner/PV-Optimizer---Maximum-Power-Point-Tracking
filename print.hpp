/*
 * print.hpp
 *
 *  Created on: 28 May 2024
 *      Author: Tobias Wallner
 *      tobias.wallner1@gmx.net
 */

#pragma once
extern "C"{
	#include <DAVE.h>
}
#include <cstring>
#include <cstdio>
#include <ostream>

inline void print(char c){
	while(UART_IsTXFIFOFull(&UART_0)){/*wait*/}
	UART_TransmitWord (&UART_0, static_cast<uint8_t>(c));
}

inline void print(const char* s, size_t count){
	for(size_t i = 0; i < count; ++i){print(s[i]);}
}

inline void print(const char* s){
	size_t count = std::strlen(s);
	print(s, count);
}

class UARTOutputStream {
public:
	friend inline UARTOutputStream& operator<<(UARTOutputStream& stream, const char* s){print(s); return stream;}
	friend inline UARTOutputStream& operator<<(UARTOutputStream& stream, char c){print(c); return stream;}
	friend inline UARTOutputStream& operator<<(UARTOutputStream& stream, bool value){return stream << ((value) ? "true" : "false");}

	template<typename Integer, std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
	friend UARTOutputStream& operator<<(UARTOutputStream& stream, Integer value){
		if(value == 0){
			return stream << '0';
		}
		char buffer[sizeof(Integer) * 8 / 3 + 1];
		char* itr = &buffer[sizeof(Integer) * 8 / 3 + 1];
		if(value < 0){
			stream << '-';
			value = -value;
		}
		uint32_t count = 0;
		while(value != 0){
			--itr;
			++count;

			Integer a = value / 10;
			Integer b = (value - a*10);
			char c = '0' + static_cast<char>(b);

			*itr = c;

			value = a;
		}
		print(itr, count);
		return stream;
	}

};

static UARTOutputStream cout;
static char endl = '\n';
