#ifndef _IRRIGATION_STSTEM_H_
#define _IRRIGATION_STSTEM_H_

#include <Arduino.h>

typedef uint64_t rawConfigData;

struct config{
    uint8_t sec         ;
    uint8_t min         ;
    uint8_t hour        ;
    uint16_t minEvent   ;
    uint16_t maxEvent   ;
};

rawConfigData toRaw( config data );
config toConfig( rawConfigData rawData );

#endif