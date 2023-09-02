#include "Searcher.h"
#include "IO.h"
#include "CRC16.h"
#include <thread>
#include <string>

using namespace std::string_literals;


void Searcher::getInputOutputInformation()
{
	std::cout << "Введите название файла дампа: "s;
	std::getline(std::cin, m_inputDumpFileName);

	std::cout << "Искать блоки с адреса (в формате 0xFFFF): "s;
	std::cin >> std::hex >> std::showbase >> m_begin;

	std::cout << "До адреса (в формате 0xFFFF): "s;
	std::cin >> m_end;

	std::cout << "Контрольная сумма (в формате 0xFFFF): "s;
	std::cin >> m_crc16;

	std::cout << "Выводить в консоль прогресс (ЗНАЧИТЕЛЬНО ЗАМЕДЛЯЕТ РАБОТУ!!!) Y/N: "s;
	char answer{};
	std::cin >> answer;
	m_log = (answer == 'Y' || answer == 'y' || answer == 'Н' || answer == 'н') ? true : false;
}

void Searcher::doJob()
{
	auto dump = IO::loadingBinaryFile<byte>(std::move(m_inputDumpFileName));
	if (m_begin > dump.size())
	{
		std::cout << "Введен несуществующий начальный адрес : " << std::hex << std::showbase << m_begin
			<< ", он будет заменен на 0x0\n";
		m_begin = 0x0;
	}
	if (m_end > dump.size())
	{
		std::cout << "Введен несуществующий конечный адрес : " << std::hex << std::showbase << m_begin
			<< ", он будет заменен на " << dump.size() << '\n';
		m_end = dump.size();
	}
	searchAdressesOfAllPossibleBlocks(std::move(dump));
}

void Searcher::searchAdressesOfAllPossibleBlocks(std::vector<byte>&& dump)
{
	const auto startBlock = dump.cbegin() + m_begin;
	const auto endBlock = dump.cbegin() + m_end + 1;

	std::atomic<size_t> progressCount{};
	std::atomic<size_t> found{};
	const size_t n = std::distance(startBlock, endBlock);
	const auto progressMax = n * n / 2;

	const size_t hardwareThreads = std::thread::hardware_concurrency();
	const auto threadNumber = std::min(hardwareThreads != 0 ? hardwareThreads : 2, n);
	const auto threadBlockSize = n / threadNumber;
	std::vector<std::thread> threads(threadNumber - 1);

	crc16Params params = { sc_polynome, sc_initValue, m_crc16 };

	const auto task =
		[atomicCount = std::ref(progressCount), found = std::ref(found), progressMax = progressMax,
		mutex = &m_mutex, consoleMutex = &m_consoleMutex, log = m_log]
	(std::vector<byte> dump, const size_t first, const size_t last, const size_t globalLast, const crc16Params params)
		{
			auto itStart = dump.cbegin() + first;
			auto itEnd = dump.cbegin() + last;
			auto itGlobalEnd = dump.cbegin() + globalLast;
			for (std::vector<byte>::const_iterator blockBegin = itStart; blockBegin != itEnd; blockBegin++)
			{
				for (auto blockEnd = blockBegin + 1; blockEnd != itGlobalEnd; blockEnd++)
				{
					if (CRC16::Calculate(blockBegin, blockEnd, params.polynome, params.initValue) == params.crc)
					{
						const auto startAddress = std::distance(dump.cbegin(), blockBegin);
						const auto endAddress = std::distance(dump.cbegin(), blockEnd) - 1;
						auto result = std::make_pair(startAddress, endAddress);
						std::unique_lock<std::recursive_mutex> lock(*mutex);
						IO::addEntryInFile(result, "results.txt"s);
						lock.unlock();
						found.get().fetch_add(1, std::memory_order_relaxed);
						std::lock_guard<std::recursive_mutex> consoleLock(*consoleMutex);
						std::cout << "\nНайден новый результат! "
							<< std::showbase << std::hex << std::uppercase << result << '\n';
						if (!log)
						{
							std::cout << '\r' << "Найдено блоков: "
								<< std::dec << found.get().load(std::memory_order_relaxed);
						}
					}
					if (log)
					{
						atomicCount.get().fetch_add(1, std::memory_order_relaxed);
						std::lock_guard<std::recursive_mutex> lock(*consoleMutex);
						IO::showProgress(atomicCount.get().load(std::memory_order_relaxed), progressMax,
							found.get().load(std::memory_order_relaxed));
					}
				}
			}
		};

	std::vector<byte>::const_iterator blockBegin = startBlock;
	for (auto currentNumber = 0; currentNumber < threadNumber - 1; currentNumber++)
	{
		std::vector<byte>::const_iterator blockEnd = blockBegin + threadBlockSize;
		threads.at(currentNumber) = std::thread(task, dump, std::distance(dump.cbegin(), blockBegin),
			std::distance(dump.cbegin(), blockEnd), std::distance(dump.cbegin(), endBlock), params);
		blockBegin = blockEnd;
	}
	task(dump, std::distance(dump.cbegin(), blockBegin), std::distance(dump.cbegin(), endBlock),
		std::distance(dump.cbegin(), endBlock), params);
	
	for (auto& entry : threads)
	{
		entry.join();
	}
}

void Searcher::Run()
{
	getInputOutputInformation();
	doJob();
}