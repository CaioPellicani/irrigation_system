#ifndef _IRRIGATION_STSTEM_H_
#define _IRRIGATION_STSTEM_H_

#include <Arduino.h>

#ifdef TEST
#include "../RTClib_EPOXY/RTClib_EPOXY.h"
#else
#include <RTClib.h>
#endif

#define MAX_CONFIG_NUM 2

/*! @protocol
*    6 bits -  0 to  5 = SEC 
*    6 bits -  6 to 11 = MIN
*    5 bits - 12 to 16 = HOUR 
*   12 bits - 17 to 28 = MIN TIME EVENT (up to 2 Hours)
*   12 bits - 29 to 40 = MAX TIME EVENT(up to 2 Hours)
*    1 bit  - 41 = type ( 0 == enqueued, 1 == simultaneous )
*/
typedef uint64_t rawConfigData;
enum { ENQUEUED, SIMULTANEOUS };

class Config{
public:
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint16_t minEvent;
    uint16_t maxEvent;
    uint8_t type;
    Config( uint8_t sec = 0, uint8_t min = 0, uint8_t hour = 0, uint16_t minEvent = 0, 
            uint16_t maxEvent = 0, uint8_t type = SIMULTANEOUS );
    Config( rawConfigData rawData );
    rawConfigData toRaw();
}; 

class Valv{
private:
    uint8_t pin;
    uint8_t configsCount;
    rawConfigData config[MAX_CONFIG_NUM];
public:  
    Valv();
    void begin( uint8_t pin );
    bool addConfig( Config newConfig );
    uint8_t getPin();
    rawConfigData getConfig( uint8_t index );
    uint8_t getCountConfig();
};

class IrSystem{
private:
    struct vNode{
        vNode* nextNode;
        Valv valv;
    };

    struct node{
        node* nextNode;
        uint8_t pin;
        uint16_t time;
        uint32_t lastMillis;
    }; 

    vNode* valvs;
    node* simultaneous;
    node* enqueued;
    bool isExecuting( uint8_t pin );
    void add( uint8_t pin, uint16_t time, uint8_t type = SIMULTANEOUS );

public:
    void remove( node** dNode );
    void addValv( uint8_t pin );
    Valv* getValv( uint8_t i ); 

    void checkConfigs( DateTime now );
    bool execute( node** eNode );
    void run( DateTime now );
};


#endif