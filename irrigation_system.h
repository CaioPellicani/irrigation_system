#ifndef _IRRIGATION_STSTEM_H_
#define _IRRIGATION_STSTEM_H_

#include <Arduino.h>

typedef uint32_t rawConfigData;
struct config{
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
};

uint32_t toRaw( config data );
config toConfig( rawConfigData rawData )

#endif