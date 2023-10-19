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
	template<typename T>
	void writeDelta(const T& data, const T& src)
	{
		const uint8_t* byteData = reinterpret_cast<const uint8_t*>(&data);
		const uint8_t* byteSrc = reinterpret_cast<const uint8_t*>(&src);

		for (int i = 0; i < sizeof(T); i++)
		{
			if (*byteData != *byteSrc)
			{
				memcpy(ptr + offsetWrite, reinterpret_cast<const uint8_t*>(&byteData), sizeof(uint8_t));
				offsetWrite += sizeof(uint8_t);
			}
		}
	}
	template<typename T>
	void findDiffMask(const T& data, const T& src)
	{
		diffMask = {};

		const uint8_t* byteData = reinterpret_cast<const uint8_t*>(&data);
		const uint8_t* byteSrc = reinterpret_cast<const uint8_t*>(&src);

		for (int i = 0; i < sizeof(T); i++)
		{
			if (*byteData == *byteSrc)
				diffMask.push_back(false);
			else
				diffMask.push_back(true);
		}
	}

	template<typename T>
	void readDelta(T& data, T& src)
	{

		for (int i = 0; i < sizeof(T); i++)
		{
			if(!diffMask.empty())
			{
				if (diffMask[0] == true)
				{
					memcpy(reinterpret_cast<uint8_t*>(&data), ptr + offsetRead, sizeof(uint8_t));
					offsetRead += sizeof(uint8_t);
				}

				diffMask.erase(diffMask.begin());
			}
		}
	}

	void readDiffMask()
	{
		uint8_t len, i;
		readRaw(len);
		diffMask = {};
		for (i = 0; i < len; i++)
		{
			if (i % 8 == 0 && i != 0)
			{
				offsetRead += sizeof(uint8_t);
			}

			diffMask.push_back(*(ptr + offsetRead) & (0x1 << (i % 8)));
		}

		if(i % 8 != 0)
			offsetRead += sizeof(uint8_t);
	}


	void writeDiffMask()
	{
		int i;
		uint8_t diffByte = 0;

		writeRaw((uint8_t)diffMask.size());

		for (i = 0; i < diffMask.size(); i++)
		{
			if(i % 8 == 0)
				diffByte = 0;

			diffByte |= 0x1 << (i % 8);

			if (i % 8 == 7)
			{
				writeRaw(diffByte);
			}
		}

		if (i % 8 != 7)
		{
			writeRaw(diffByte);
		}
	}

private:
		uint8_t* ptr;
		uint32_t offsetWrite = 0;
		uint32_t offsetRead = 0;
		std::vector<bool> diffMask = {};
};