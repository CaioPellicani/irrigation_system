#include "irrigation_system.h"

#define NUM_VALV 2
Valv arrValv[NUM_VALV];

void setup(){
    Serial.begin( 9600 );
    while( !rtc.begin() );
    if( rtc.lostPower() ){
        #ifdef TEST
        rtc.adjust( DateTime(2022, 2, 8, 0, 0, 0 ) );
        #else
        rtc.adjust( DateTime( F( __DATE__ ), F( __TIME__ ) ) );
        #endif
    }
    Serial.print( "RTC Begin at: " );
    Serial.println( rtc.now().timestamp( DateTime::TIMESTAMP_FULL ) );

    arrValv[0].begin( 7 );
    arrValv[1].begin( 8 );

    arrValv[0].addConfig( 9, 0, 0, 1, 5 );  
    arrValv[1].addConfig( 9, 0, 1, 1, 5 );  
}

void loop(){
    for( int i = 0; i < NUM_VALV; i++ ){
        Serial.print( "Valv " + String( i ) + " ");
        arrValv[i].checkConfigs();
    }
#ifdef TEST
    static int i = 0;
    if( ++i >= 3 ){ exit(0); }
#endif
}   

void Valv::begin( uint8_t pin ){
    this->configsCount = 0;
    this->pin = pin;
    pinMode( this->pin, OUTPUT );
    digitalWrite( this->pin, LOW );
}

void Valv::checkConfigs(){
    for( int i = 0; i < this->configsCount; i++ ){
        Serial.println( this->config[i] );
    }
}

bool Valv::addConfig( uint8_t hour, uint8_t min, uint8_t sec, uint16_t minEvent, uint16_t maxEvent ){
    if( this->configsCount < MAX_CONFIG_NUM ){
        this->config[ this->configsCount++ ] = this->toRaw( Valv::Config{
            .sec = sec,
            .min = min,
            .hour = hour,
            .minEvent = minEvent,
            .maxEvent = maxEvent
        });
        return true;
    }
    return false;
}

rawConfigData Valv::getConfig( uint8_t index ){
    return this->config[ index ];
}
uint8_t Valv::getCountConfig(){
    return this->configsCount;
}

/*! @protocol
*    6 bits -  0 to  5 = SEC 
*    6 bits -  6 to 11 = MIN
*    5 bits - 12 to 16 = HOUR 
*   12 bits - 17 to 28 = MIN TIME EVENT (up to 2 Hours)
*   12 bits - 29 to 40 = MAX TIME EVENT(up to 2 Hours)
*/
rawConfigData Valv::toRaw( Valv::Config data ){
    while( data.sec >= 60 ){
        data.sec -= 60;
        data.min++;
    }
    while( data.min >= 60 ){
        data.min -= 0;
        data.hour++;
    }   
    while( data.hour >= 24 ){ 
       data.hour -= 24; }

    if( data.minEvent > 0xFFF ) data.minEvent = 0xFFF;
    if( data.maxEvent > 0xFFF ) data.maxEvent = 0xFFF;

    return ( 
        ( ( uint64_t ) data.sec ) | 
        ( ( uint64_t ) data.min      <<  6 ) |
        ( ( uint64_t ) data.hour     << 12 ) |  
        ( ( uint64_t ) data.minEvent << 17 ) | 
        ( ( uint64_t ) data.maxEvent << 29 ) );
}

Valv::Config Valv::read( rawConfigData rawData ){
    return Valv::Config{
        .sec =      ( uint8_t ) (    rawData & ( uint64_t ) 0x3F ),
        .min =      ( uint8_t ) ( (  rawData & ( uint64_t ) 0x3F  <<  6 ) >>  6 ),
        .hour =     ( uint8_t ) ( (  rawData & ( uint64_t ) 0x1F  << 12 ) >> 12 ),
        .minEvent = ( uint16_t ) ( ( rawData & ( uint64_t ) 0xFFF << 17 ) >> 17 ),
        .maxEvent = ( uint16_t ) ( ( rawData & ( uint64_t ) 0xFFF << 29 ) >> 29 )
    };
}