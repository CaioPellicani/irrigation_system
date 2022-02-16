#include "IrSystem.h"

RTC_DS3231 rtc;
IrSystem irSys;

void setup(){
    Serial.begin( 9600 );
    while( !rtc.begin() );
    if( rtc.lostPower() ){
        #ifdef TEST
        rtc.adjust( DateTime(2022, 2, 8, 9, 0, 0 ) );
        #else
        rtc.adjust( DateTime( F( __DATE__ ), F( __TIME__ ) ) );
        #endif
    }
    Serial.print( "RTC Begin at: " );
    Serial.println( rtc.now().timestamp( DateTime::TIMESTAMP_FULL ) );

    irSys.addValv( 7 );
    irSys.addValv( 8 );
    irSys.addValv( 9 );

    irSys.getValv( 7 )->addConfig( Config( 9, 0, 0, 1, 2, SIMULTANEOUS ) );
    irSys.getValv( 7 )->addConfig( Config( 9, 0, 7, 1, 2, SIMULTANEOUS ) );
    irSys.getValv( 8 )->addConfig( Config( 9, 0, 2, 1, 2, SIMULTANEOUS ) ); 
    irSys.getValv( 9 )->addConfig( Config( 9, 0, 1, 1, 2, SIMULTANEOUS ) ); 
}


void loop(){
    Serial.println( rtc.now().timestamp( DateTime::TIMESTAMP_FULL ) );
    Serial.print( "pin 7 -> ");
    Serial.println( digitalRead( 7 ) );
    Serial.print( "pin 8 -> ");
    Serial.println( digitalRead( 8 ) );
    Serial.print( "pin 9 -> ");
    Serial.println( digitalRead( 9 ) );

    irSys.run( rtc.now() );
    delay( 1000 );
//#ifdef TEST
    static int i = 10; if( --i <= 0 ) exit(0); 
    Serial.print( "i = " );
    Serial.println( 10-i );
//#endif

}   
