#ifndef _IRRIGATION_STSTEM_H_
#define _IRRIGATION_STSTEM_H_

#include <Arduino.h>

#ifdef TEST
#include "../RTClib_EPOXY/RTClib_EPOXY.h"
#else
#include <RTClib.h>
#endif

#define MAX_CONFIG_NUM 2
RTC_DS3231 rtc;

typedef uint64_t rawConfigData;

class Valv{
private:
    uint8_t pin;
    uint8_t configsCount;
    rawConfigData config[MAX_CONFIG_NUM];

    struct Config{
        uint8_t sec;
        uint8_t min;
        uint8_t hour;
        uint16_t minEvent;
        uint16_t maxEvent;
    };
    rawConfigData toRaw( Config data );
    Config read( rawConfigData rawData ); 
public:
    void begin( uint8_t pin );
    bool addConfig( uint8_t sec = 0, 
                    uint8_t min = 0, 
                    uint8_t hour = 0, 
                    uint16_t minEvent = 0, 
                    uint16_t maxEvent = 0 );
    rawConfigData getConfig( uint8_t index );
    uint8_t getCountConfig();
    void checkConfigs();
};


#endif