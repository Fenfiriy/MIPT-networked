#pragma once
#include <cstring>
#include "quantisation.h"

class Bitstream
{
public:
	Bitstream(uint8_t* data) : ptr(data) {}
	template<typename T>
	void writeQuantized(const T& data)
	{

		writeRaw();
	}
	template<typename T>
	void readQuantized(T& data)
	{
		readRaw();
	}
	template<typename T>
	void writeRaw(const T& data)
	{
		memcpy(ptr + offsetWrite, reinterpret_cast<const uint8_t*>(&data), sizeof(T));
		offsetWrite += sizeof(T);
	}
	template<typename T>
	void readRaw(T& data)
	{
		memcpy(reinterpret_cast<uint8_t*>(&data), ptr + offsetRead, sizeof(T));
		offsetRead += sizeof(T);
	}

	void setDeltaSource(const Bitstream& bs)
	{

	}

private:
		uint8_t* ptr;
		uint32_t offsetWrite = 0;
		uint32_t offsetRead = 0;
};