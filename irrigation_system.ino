#include "IrSystem.h"

RTC_DS3231 rtc;
IrSystem irSys;

void setup(){
    Serial.begin( 9600 );
    while( !rtc.begin() );
    if( rtc.lostPower() ){
        #ifdef TEST
        rtc.adjust( DateTime(2022, 2, 11, 9, 0, 0 ) );
        #else
        rtc.adjust( DateTime( F( __DATE__ ), F( __TIME__ ) ) );
        #endif
    }
    Serial.print( "RTC Begin at: " );
    Serial.println( rtc.now().timestamp( DateTime::TIMESTAMP_FULL ) );

    irSys.monthlyPercent(  16.69, 30.66, 51.82, 82.12, 91.97, 93.43, 
                          100.00, 99.64, 89.05, 59.49, 40.51, 10.22 );

    irSys.addValv( 7 );
    irSys.addValv( 8 );
    irSys.addValv( 9 );

    irSys.getValv( 7 )->addConfig( Config( 9, 0, 0, ( 5 * 60 * 60 ), true, 0, ENQUEUED ) );
    irSys.getValv( 7 )->addConfig( Config( 9, 0, 7, ( 5 * 60 * 60 ), true, 0, ENQUEUED ) );
    irSys.getValv( 8 )->addConfig( Config( 9, 0, 2, ( 5 * 60 * 60 ), true, 0, ENQUEUED ) ); 
    irSys.getValv( 9 )->addConfig( Config( 9, 0, 1, ( 5 * 60 * 60 ), true, 0, ENQUEUED ) ); 
    float x = 22.88000000;
    Serial.println( (int)( 100 * x ) );
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
