#include "IrSystem.h"

RTC_DS3231 rtc;
IrSystem irSys;

void setup(){
    Serial.begin( 9600 );
    while( !rtc.begin() );
    if( rtc.lostPower() ){
        #ifdef TEST
        rtc.adjust( DateTime(2022, 2, 10, 9, 0, 0 ) );
        #else
        rtc.adjust( DateTime( F( __DATE__ ), F( __TIME__ ) ) );
        #endif
    }
    Serial.print( "RTC Begin at: " );
    Serial.println( rtc.now().timestamp( DateTime::TIMESTAMP_FULL ) );

    irSys.monthlyPercent( 16, 31, 52, 82, 92, 93, 100, 100, 89, 59, 41, 10 );

    irSys.addValv( 7 );
    irSys.addValv( 8 );
    irSys.addValv( 9 );

    irSys.getValv( 7 )->addConfig( Config( 9, 0, 0, 5, true, 0, ENQUEUED ) );
    irSys.getValv( 7 )->addConfig( Config( 9, 0, 7, 5, true, 0, ENQUEUED ) );
    irSys.getValv( 8 )->addConfig( Config( 9, 0, 2, 5, true, 0, ENQUEUED ) ); 
    irSys.getValv( 9 )->addConfig( Config( 9, 0, 1, 5, true, 0, ENQUEUED ) ); 
}

void loop(){
    irSys.run( rtc.now() );
    static uint32_t lastMillis = millis();
    if( millis() - lastMillis >= 1000 ){
        digitalWrite( LED_BUILTIN, !digitalRead( LED_BUILTIN ) );
    }

    static int map[3] = { LOW, LOW, LOW };
    for( int i = 0; i < 3; i++ ){
        if( digitalRead( i + 7 ) != map[i] ){
            map[i] = digitalRead( i + 7 );
            Serial.println( rtc.now().timestamp( DateTime::TIMESTAMP_FULL ) );
            Serial.print( i + 7 );
            Serial.print( " -> " );
            Serial.println( map[i] );
            #ifdef TEST
            static int interations = 8; if( --interations <= 0 ) exit(0); 
            #endif
        }
    }


}   
