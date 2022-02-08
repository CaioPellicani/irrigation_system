#include "irrigation_system.h"

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

    rawConfigData value = toRaw( config{ 
            .sec=5, 
            .min=6, 
            .hour=7, 
            .minEvent=5*60, 
            .maxEvent=20*60
        } 
    );

    Serial.println( value, BIN );
    Serial.println( toConfig( value ).sec );
    Serial.println( toConfig( value ).min );
    Serial.println( toConfig( value ).hour );
    Serial.println( toConfig( value ).minEvent );
    Serial.println( toConfig( value ).maxEvent );
}

void loop(){}   

rawConfigData toRaw( config data ){
    /*  
    PROTOCOL:
     6 bits -  0 to  5 = SEC
     6 bits -  6 to 11 = MIN
     5 bits - 12 to 16 = HOUR 
    12 bits - 17 to 28 = MIN TIME EVENT (up to 2 Hours)
    12 bits - 29 to 40 = MAX TIME EVENT(up to 2 Hours)
    */

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

config toConfig( rawConfigData rawData ){
    return {
        .sec =      ( uint8_t ) (    rawData & ( uint64_t ) 0x3F ),
        .min =      ( uint8_t ) ( (  rawData & ( uint64_t ) 0x3F  <<  6 ) >>  6 ),
        .hour =     ( uint8_t ) ( (  rawData & ( uint64_t ) 0x1F  << 12 ) >> 12 ),
        .minEvent = ( uint16_t ) ( ( rawData & ( uint64_t ) 0xFFF << 17 ) >> 17 ),
        .maxEvent = ( uint16_t ) ( ( rawData & ( uint64_t ) 0xFFF << 29 ) >> 29 )
    };
}