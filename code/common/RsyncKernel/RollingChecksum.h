//
// Created by carrot on 18-3-23.
//

#ifndef RSYNCTOOL_ROLLINGCHECKSUM_H
#define RSYNCTOOL_ROLLINGCHECKSUM_H


#include <cstdint>

class RollingChecksum
{
public:
    static uint32_t adler32_checksum(char *buf, int len);

    static uint32_t adler32_rolling_checksum(uint32_t lastchecksum, int len, char c1, char c2);
};


#endif //RSYNCTOOL_ROLLINGCHECKSUM_H
