#ifndef __IR_SYSTEM_H__
#define __IR_SYSTEM_H__

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
*    6 bits - 17 to 22 = PAUSE AFTER EXECUTION ( up to 60 sec)
*   12 bits - 23 to 34 = TIME EVENT (up to 2 Hours)
*    1 bit  - 35       = USE monthlyPercent
*    1 bit  - 36       = type ( 0 == enqueued, 1 == simultaneous )
*/
typedef uint64_t rawConfigData;
enum { ENQUEUED, SIMULTANEOUS };

class Config{
public:
    uint8_t sec;
    uint8_t min;
    uint8_t hour;
    uint8_t pause;
    uint16_t secHIGH;
    bool useMonthlyPercent;
    uint8_t type;
    Config(){};
    Config( uint8_t hour, uint8_t min, uint8_t, uint16_t secHIGH = 0, bool useMonthlyPercent = false, 
            uint8_t pause = 0, uint8_t type = SIMULTANEOUS );
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
        uint8_t pause;
        uint32_t lastMillis;
    }; 

    vNode* valvs;
    node* simultaneous;
    node* enqueued;
    uint8_t arrMonthlyPercent[12];
    bool isExecuting( uint8_t pin );
    void add( uint8_t pin, uint16_t time, uint8_t pause, uint8_t type );

public:
    IrSystem();
    void remove( node** dNode );
    void addValv( uint8_t pin );
    Valv* getValv( uint8_t i ); 
    void monthlyPercent( uint8_t jan, uint8_t feb, uint8_t mar, uint8_t apr, 
                            uint8_t may, uint8_t jun, uint8_t jul, uint8_t ago, 
                            uint8_t sep, uint8_t out, uint8_t nov, uint8_t dez );
    void checkConfigs( DateTime now );
    void execute( node** eNode );
    void run( DateTime now );
};

#endif