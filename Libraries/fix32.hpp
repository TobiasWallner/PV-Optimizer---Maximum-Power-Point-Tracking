/*
	Author: Tobias Wallner
	tobias.wallner1@gmx.net
	
*/

#pragma once

#include <cinttypes>

#include <bitset>

template<size_t fractional_bits>
class fix32{
private:
	int32_t value;
	
public:
	inline fix32() = default;
	inline fix32(const fix32&) = default;
	inline fix32(int32_t number) : value(number << fractional_bits){}
	inline fix32(uint32_t number) : fix32(static_cast<int32_t>(number)){}
	
	template<typename Integer, std::enable_if_t<std::is_integral<Integer>::value, bool> = true>
	fix32(Integer number) : fix32(static_cast<int32_t>(number)){}

	inline fix32(float num) {
		if (num != 0.f) {
			const int32_t inum = *reinterpret_cast<const uint32_t*>(&num);
			const int32_t float_mantissa = (inum & 0x07FFFFF) | 0x0800000;
			const int32_t float_exponent = ((inum & 0x7F800000) >> 23) - 127;
			const bool float_sign = (*reinterpret_cast<int32_t*>(&num) & 0x80000000) != 0;
			const int32_t shifts = fractional_bits - 23 + float_exponent;
			const int32_t abs_value = (shifts >= 0) ? float_mantissa << shifts : float_mantissa >> -shifts;
			this->value = (float_sign) ? -abs_value : abs_value;
		}
		else {
			this->value = 0;
		}
	}
	
	inline fix32(double num) : fix32(static_cast<float>(num)){}
	
	template<size_t other_frac_bits>
	inline fix32(const fix32<other_frac_bits>& other){
		if (fractional_bits >= other_frac_bits)
			this->value = other.reinterpret_as_int32_t() << static_cast<size_t>(fractional_bits - other_frac_bits);
		else
			this->value = other.reinterpret_as_int32_t() >> static_cast<size_t>(other_frac_bits - fractional_bits);
	}
	
	inline fix32& operator= (const fix32&) = default;
	
	// Arithmetic operators
	
	inline friend fix32 operator+ (fix32 lhs, fix32 rhs){
		fix32 result;
		result.value = lhs.value + rhs.value;
		return result;
	}
	
	inline friend fix32 operator- (fix32 a){
		fix32 result;
		result.value = -a.value;
		return result;
	}
	
	inline friend fix32 operator- (fix32 lhs, fix32 rhs){
		fix32 result;
		result.value = lhs.value - rhs.value;
		return result;
	}
	
	inline friend fix32 operator* (fix32 lhs, fix32 rhs){
		int64_t temp = static_cast<int64_t>(lhs.value) * static_cast<int64_t>(rhs.value);
		return fix32::reinterpret(static_cast<int32_t>(static_cast<uint32_t>(temp >> fractional_bits)));
	}
	
	inline friend fix32 operator/ (fix32 lhs, fix32 rhs){
		int64_t temp = (static_cast<int64_t>(lhs.value) << fractional_bits) / static_cast<int64_t>(rhs.value);
		return fix32::reinterpret(static_cast<int32_t>(temp));
	}
	
	inline fix32& operator+= (fix32 rhs){return *this = *this + rhs;}
	inline fix32& operator-= (fix32 rhs){return *this = *this - rhs;}
	inline fix32& operator*= (fix32 rhs){return *this = *this * rhs;}
	inline fix32& operator/= (fix32 rhs){return *this = *this / rhs;}
	
	// Comparison operators
	inline friend bool operator== (fix32 lhs, fix32 rhs){return lhs.value == rhs.value;}
	inline friend bool operator!= (fix32 lhs, fix32 rhs){return lhs.value != rhs.value;}
	inline friend bool operator< (fix32 lhs, fix32 rhs){return lhs.value < rhs.value;}
	inline friend bool operator> (fix32 lhs, fix32 rhs){return lhs.value > rhs.value;}
	inline friend bool operator<= (fix32 lhs, fix32 rhs){return lhs.value <= rhs.value;}
	inline friend bool operator>= (fix32 lhs, fix32 rhs){return lhs.value >= rhs.value;}
	
	static inline fix32 reinterpret(int32_t number){
		fix32 result;
		result.value = number;
		return result;
	}
	
	template<size_t other_frac_bits>
	explicit inline operator fix32<other_frac_bits> () {return fix32<other_frac_bits>(*this);}
	
	explicit inline operator int32_t (){return this->value >> fractional_bits;}
	explicit inline operator uint32_t (){return static_cast<uint32_t>(this->value >> fractional_bits);}
	
	explicit inline operator float (){
		 static float fnorm = 1.f / static_cast<float>(1 << fractional_bits);
		float fval = static_cast<float>(this->value);
		float result = fval * fnorm;
		return result;
	}
	
	explicit inline operator double (){
		 static double fnorm = 1.f / static_cast<double>(1 << fractional_bits);
		double fval = static_cast<double>(this->value);
		double result = fval * fnorm;
		return result;
	}
	
	inline int32_t static_cast_to_int32_t() const {return this->value >> fractional_bits;}
	inline int32_t reinterpret_as_int32_t() const {return this->value;}
	
	inline friend int32_t static_cast_to_int32_t(fix32 f){return f.value >> fractional_bits;}
	inline friend int32_t reinterpret_as_int32_t(fix32 f){return f.value;}
	
	template<class Stream>
	friend Stream& operator<<(Stream& stream, fix32 f){
		if(f < 0){
			stream << '-';
			f = -f;
		}
		uint32_t digits = f.value >> fractional_bits;
		uint32_t fractionals = f.value & ((1 << fractional_bits) - 1);

		int significant_places = 0;
		bool count_significant_enable = digits != 0;

		// print digits and decimal point
		stream << digits << '.';

		// print fractionals
		while(fractionals != 0 && significant_places < 3){
			significant_places += count_significant_enable;
			fractionals = fractionals * 10;
			uint32_t n = fractionals >> fractional_bits;
			count_significant_enable |= n != 0;
			fractionals = fractionals & ((1 << fractional_bits) - 1);
			char c = '0' + static_cast<char>(n);
			stream << c;
		}
		return stream;
	}
};
