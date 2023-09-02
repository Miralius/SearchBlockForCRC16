#pragma once

#include <stdint.h>
#include <vector>

class CRC16
{
public:
    using byte = uint8_t;
    static inline uint16_t Calculate(const std::vector<byte>::const_iterator& begin,
        const std::vector<byte>::const_iterator& end, const uint16_t polynome, const uint16_t initValue)
    {
        auto crc = initValue;
        for (std::vector<byte>::const_iterator it = begin; it != end; it++)
        {
            crc ^= (*it << 8);
            for (uint8_t j{}; j < 8; j++)
            {
                if ((crc & 0x8000) != 0)
                {
                    crc = crc << 1 ^ polynome;
                }
                else
                {
                    crc <<= 1;
                }
            }
        }
        return crc;
    }
};

