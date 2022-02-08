#include "irrigation_system.h"

void setup(){
    Serial.begin( 9600 );
    rawConfigData value = toRaw( config{ .sec=63, .min=0, .hour=0 } );
    Serial.println( value, BIN );
    Serial.println( toConfig( value ).sec );
    Serial.println( toConfig( value ).min );
    Serial.println( toConfig( value ).hour );
}

void loop(){}

uint32_t toRaw( config data ){
    /*  
    PROTOCOL:
    bits  0 to  5 = SEC
    bits  6 to 11 = MIN
    bits 12 to 16 = HOUR  
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
    return ( data.hour << 12 ) | ( data.min << 6 ) | data.sec;
}

config toConfig( rawConfigData rawData ){
    return {
        .sec =  ( uint8_t )   ( rawData & 0x3F            ),
        .min =  ( uint8_t ) ( ( rawData & 0xFC0   ) >> 6  ),
        .hour = ( uint8_t ) ( ( rawData & 0x1F000 ) >> 12 )
    };
}