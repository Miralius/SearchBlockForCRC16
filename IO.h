#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <iomanip>

class IO
{
public:
	template <typename T>
	static inline std::vector<T> loadingBinaryFile(std::string&& nameFile) {
		std::ifstream in(nameFile, std::ios::binary);
        std::vector<T> binaryDataVector{};
        if (!in.fail())
        {
            T binaryData{};
            while (in >> std::noskipws >> binaryData)
            {
                binaryDataVector.emplace_back(binaryData);
            }
            in.close();
        }
        else
        {
            in.close();
            throw std::runtime_error(std::move(nameFile) + " is not found!");
        }
        if (binaryDataVector.size() == 0)
        {
            in.close();
            throw std::runtime_error(std::move(nameFile) + " is empty or contains wrong data!");
        }
		return binaryDataVector;
	}

    static void showProgress(const size_t completed, const size_t total, const size_t found)
    {
        auto percent = std::trunc(10000 * (static_cast<float>(completed) / total)) / 100;
        std::cout << '\r' << "Выполнено: " << std::dec << std::setw(6) << percent << "%, найдено блоков: " << found << "   ";
    }

    template <class T>
    static void addEntryInFile(T&& entry, std::string&& nameFile)
    {
        std::ofstream output(nameFile, std::ios::app);
        if (!output)
        {
            output.close();
            throw std::runtime_error("Writing into file " + std::move(nameFile) + " is impossible!");
        }
        output << std::showbase << std::hex << std::uppercase << std::move(entry) << '\n';
        output.close();
    }
};

template <class T, class U>
inline std::ostream& operator<<(std::ostream& out, const std::pair<T, U>& obj)
{
    return out << '[' << obj.first << ", " << obj.second << ']';
}