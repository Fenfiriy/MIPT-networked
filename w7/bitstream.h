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
	void writePackedUint(T value)
	{
		if (value < 0x80) // = 2^7 = 128
		{
			const auto data = uint8_t{ value };
			memcpy(offsetWrite + ptr, reinterpret_cast<const uint8_t*>(&data), sizeof(uint8_t));
			offsetWrite += sizeof(uint8_t);
		}

		else if (value < 0x4'000) // = 2^14 = 16'384
		{
			const auto data = reverse_if_little_endian(uint16_t{ (1 << 15) | value });
			memcpy(offsetWrite + ptr, reinterpret_cast<const uint16_t*>(&data), sizeof(uint16_t));
			offsetWrite += sizeof(uint16_t);
		}

		else (value < 0x40'000'000) // = 2^30 = 1'073'741'824
		{
			const auto data = reverse_if_little_endian(uint32_t{ (3 << 30) | value });
			memcpy(offsetWrite + ptr, reinterpret_cast<const uint32_t*>(&data), sizeof(uint32_t));
			offsetWrite += sizeof(uint32_t);
		}
	}
	template<typename T>
	void readPackedUint(T& value) {
		uint8_t type = *(uint8_t*)(ptr + offsetRead);

		switch (type >> 6)
		{
		case 0:
		case 1:
			value = type;
			offsetRead += sizeof(uint8_t);
			break;
		case 2:
			value = reverse_if_little_endian(*(uint16_t*)(ptr + offsetRead)) & 0x3FFF;
			offsetRead += sizeof(uint16_t);
			break;
		default:
			value = reverse_if_little_endian(*(uint32_t*)(ptr + offsetRead)) & 0x3FFFFFFF;
			offsetRead += sizeof(uint32_t);
			break;
		}
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

	static bool isBigEndian() {
		uint16_t word = 1; // 0x0001
		uint8_t* first_byte = (uint8_t*)&word; // points to the first byte of word
		return !(*first_byte); // true if the first byte is zero
	}


	static uint16_t reverse_if_little_endian(uint16_t num) {
		if (isBigEndian())
			return num;

		uint8_t c1, c2;
		c1 = num & 255;
		c2 = (num >> 8) & 255;
		return (c1 << 8) + c2;
	}
private:
	uint8_t* ptr;
	size_t offsetWrite = 0;
	size_t offsetRead = 0;
};