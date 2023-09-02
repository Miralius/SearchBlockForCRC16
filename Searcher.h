#pragma once

#include <iostream>
#include <vector>
#include <mutex>

class Searcher
{
public:
	using byte = uint8_t;
	void Run();
private:
	struct crc16Params {
		uint16_t polynome;
		uint16_t initValue;
		uint16_t crc;
	};

	void getInputOutputInformation();
	void doJob();
	void searchAdressesOfAllPossibleBlocks(std::vector<byte>&& dump);

	std::recursive_mutex m_mutex;
	std::recursive_mutex m_consoleMutex;
	std::string m_inputDumpFileName;
	size_t m_begin;
	size_t m_end;
	uint16_t m_crc16;
	bool m_log;
	static constexpr uint16_t sc_polynome = 0x1021;
	static constexpr uint16_t sc_initValue = 0xFFFF;
};