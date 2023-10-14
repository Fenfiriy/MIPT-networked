#pragma once
#include "mathUtils.h"
#include <limits>

struct vec2
{
	float x;
	float y;
};

struct vec2int
{
	uint8_t x;
	uint8_t y;
};

inline bool operator==(const vec2int& lhs, const vec2int& rhs) {
	return lhs.x == rhs.x && lhs.y == rhs.y;
}
inline bool operator!=(const vec2int& lhs, const vec2int& rhs) {
	return !(lhs == rhs);
}

struct vec3
{
	float x;
	float y;
	float z;
};

template<typename T>
T pack_float(float v, float lo, float hi, int num_bits)
{
  T range = (1 << num_bits) - 1;//std::numeric_limits<T>::max();
  return range * ((clamp(v, lo, hi) - lo) / (hi - lo));
}

template<typename T>
float unpack_float(T c, float lo, float hi, int num_bits)
{
  T range = (1 << num_bits) - 1;//std::numeric_limits<T>::max();
  return float(c) / range * (hi - lo) + lo;
}

template<typename T, int num_bits>
class QuantizedFloat
{
  T packedVal;

  QuantizedFloat(float v, float lo, float hi) { pack(v, lo, hi); }
  QuantizedFloat(T compressed_val) : packedVal(compressed_val) {}

  void pack(float v, float lo, float hi) { packedVal = pack_float<T>(v, lo, hi, num_bits); }
  float unpack(float lo, float hi) { return unpack_float<T>(packedVal, lo, hi, num_bits); }
};

template <typename T, int num_bits1, int num_bits2>
class QuantizedVec2
{
	T packedval;

	QuantizedVec2(float v1, float v2, float lo, float hi) { pack(v1, v2, lo, hi); }
	QuantizedVec2(T compressed_val) : packedval(compressed_val) {}

	void pack(float v1, float v2, float lo, float hi)
	{
		const T packedval1 = pack_float<T>(v1, lo, hi, num_bits1);
		const T packedval2 = pack_float<T>(v2, lo, hi, num_bits2);

		packedval = packedval1 << num_bits2 | packedval2;
	}
	vec2 unpack(float lo, float hi)
	{
		vec2 vect;
		vect.x = unpack_float<T>(packedval >> num_bits2, lo, hi, num_bits1);
		vect.y = unpack_float<T>(packedval & ((1 << num_bits2) - 1), lo, hi, num_bits2);
		return vect;
	}
};

template <typename T, int num_bits1, int num_bits2, int num_bits3>
class QuantizedVec3
{
	T packedval;

	QuantizedVec3(float v1, float v2, float v3, float lo, float hi) { pack(v1, v2, v3, lo, hi); }
	QuantizedVec3(T compressed_val) : packedval(compressed_val) {}

	void pack(float v1, float v2, float v3, float lo, float hi)
	{
		const T packedval1 = pack_float<T>(v1, lo, hi, num_bits1);
		const T packedval2 = pack_float<T>(v2, lo, hi, num_bits2);
		const T packedval3 = pack_float<T>(v3, lo, hi, num_bits3);

		packedval = (packedval1 << (num_bits2 + num_bits3)) | (packedval2 << num_bits3) | packedval3;
	}
	vec2 unpack(float lo, float hi)
	{
		vec3 vect;
		vect.x = unpack_float<T>(packedval >> num_bits2, lo, hi, num_bits1);
		vect.y = unpack_float<T>((packedval >> num_bits3) & ((1 << num_bits2) - 1), lo, hi, num_bits2);
		vect.z = unpack_float<T>(packedval & ((1 << num_bits3) - 1), lo, hi, num_bits3);
		return vect;
	}
};




typedef QuantizedFloat<uint8_t, 4> float4bitsQuantized;