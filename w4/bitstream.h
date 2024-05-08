#pragma once
#include <cstring>
#include <cstdint>

class Bitstream
{
public:
	Bitstream(uint8_t* data) : ptr(data) {}
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
	template<typename T>
	void skip()
	{
		offsetRead += sizeof(T);
	}
	void reset()
	{
		offsetWrite = 0;
		offsetRead = 0;
	}


private:
	uint8_t* ptr;
	size_t offsetWrite = 0;
	size_t offsetRead = 0;
};