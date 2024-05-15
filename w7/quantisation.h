#pragma once
#include "mathUtils.h"
#include <limits>

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
struct PackedFloat
{
  T packedVal;

  PackedFloat(float v, float lo, float hi) { pack(v, lo, hi); }
  PackedFloat(T compressed_val) : packedVal(compressed_val) {}

  void pack(float v, float lo, float hi) { packedVal = pack_float<T>(v, lo, hi, num_bits); }
  float unpack(float lo, float hi) { return unpack_float<T>(packedVal, lo, hi, num_bits); }
};

typedef PackedFloat<uint8_t, 4> float4bitsQuantized;

struct float2
{
    float x;
    float y;

    float2(float a, float b) : x(a), y(b) {}
};

struct int2
{
	int x;
	int y;

	int2(int a, int b) : x(a), y(b) {}
};

struct float3
{
    float x;
    float y;
    float z;

    float3(float a, float b, float c) : x(a), y(b), z(c) {}
};

struct int3
{
	int x;
	int y;
	int z;

	int3(int a, int b, int c) : x(a), y(b), z(c) {}
};

template<typename T, int num_bitsX, int num_bitsY>
struct PackedFloat2
{
    T packedVal;

    PackedFloat2(float2 v, float2 lo, float2 hi) { pack(v, lo, hi); }
    PackedFloat2(T compressed_val) : packedVal(compressed_val) {}

    void pack(float2 v, float2 lo, float2 hi)
    {
        T packedValX = pack_float<T>(v.x, lo.x, hi.x, num_bitsX);
        T packedValY = pack_float<T>(v.y, lo.y, hi.y, num_bitsY);
        packedVal = (packedValX << num_bitsY) | packedValY;
    }
    float unpack(float2 lo, float2 hi)
    {
        return float2(unpack_float<T>(packedVal >> num_bitsY, lo.x, hi.x, num_bitsX), unpack_float<T>(packedVal | ((1 << num_bitsY) - 1), lo.y, hi.y, num_bitsY));
    }
};

template<typename T, int num_bitsX, int num_bitsY, int num_bitsZ>
struct PackedFloat3
{
	T packedVal;

	PackedFloat3(float3 v, float3 lo, float3 hi) { pack(v, lo, hi); }
	PackedFloat3(T compressed_val) : packedVal(compressed_val) {}

    void pack(float3 v, float3 lo, float3 hi)
    {
		T packedValX = pack_float<T>(v.x, lo.x, hi.x, num_bitsX);
		T packedValY = pack_float<T>(v.y, lo.y, hi.y, num_bitsY);
		T packedValZ = pack_float<T>(v.z, lo.z, hi.z, num_bitsZ);
		packedVal = (packedValX << (num_bitsY + num_bitsZ)) | (packedValY << num_bitsZ) | packedValZ;
	}
    float unpack(float3 lo, float3 hi)
    {
		return float3(unpack_float<T>(packedVal >> (num_bitsY + num_bitsZ), lo.x, hi.x, num_bitsX), unpack_float<T>((packedVal >> num_bitsZ) & ((1 << num_bitsY) - 1), lo.y, hi.y, num_bitsY), unpack_float<T>(packedVal & ((1 << num_bitsZ) - 1), lo.z, hi.z, num_bitsZ));
	}
};