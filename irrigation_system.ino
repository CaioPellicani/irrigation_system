#include "IrSystem.h"

#define EXTERNAL_ARDUINO
//#define INTERNAL_ARDUINO

#define VALV_INTERNAL 8
#define VALV_FRONT 6 
#define VALV_BACK 5

enum groups{ INTERNAL_GARDEN, EXTERNAL_GARDEN };

RTC_DS3231 rtc;
IrSystem irSys;

void setup(){
    Serial.begin( 9600 );
    while( !rtc.begin() );
    if( rtc.lostPower() ){
#ifdef TEST
        rtc.adjust( DateTime(2022, 2, 11, 13, 0, 0 ) );
#else
        rtc.adjust( DateTime( F( __DATE__ ), F( __TIME__ ) ) );
        Serial.print( "Adjusting RTC" );
#endif
    }
    Serial.print( "RTC Begin at: " );
    Serial.println( rtc.now().timestamp( DateTime::TIMESTAMP_FULL ) );
    
    irSys.monthlyPercent(  16.69, 30.66, 51.82, 82.12, 91.97, 93.43, 
                          100.00, 99.64, 89.05, 59.49, 40.51, 10.22 );

#ifdef TEST
    uint16_t fiveMin = 5;
#else
    uint16_t fiveMin = 5 * 60 * 60;
#endif

    irSys.addValv( VALV_INTERNAL, INTERNAL_GARDEN )->addConfig( 
        Config( 9, 0, 0, 
                fiveMin, 
                false, 
                10, 
                ENQUEUED 
        ) 
    );
    irSys.addValv( VALV_FRONT, EXTERNAL_GARDEN )->addConfig( 
        Config( 9, 0, 1, 
                fiveMin, 
                true, 
                10, 
                ENQUEUED 
        ) 
    );
    irSys.addValv( VALV_BACK, EXTERNAL_GARDEN )->addConfig( 
        Config( 9, 0, 2, 
                fiveMin, 
                true, 
                10, 
                ENQUEUED 
        ) 
    );
#ifdef EXTERNAL_ARDUINO
    irSys.deactivateGroup( INTERNAL_GARDEN );
#endif
#ifdef INTERNAL_ARDUINO
    irSys.deactivateGroup( EXTERNAL_GARDEN );
#endif
}

void loop(){
    irSys.run( rtc.now() );
    
    static uint32_t lastMillis = millis();
    if( millis() - lastMillis >= 1000 ){
        digitalWrite( LED_BUILTIN, !digitalRead( LED_BUILTIN ) );
        lastMillis = millis();
    }

#ifdef TEST
    static int map[14] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    for( int i = 0; i < 14; i++ ){
        if( digitalRead( i ) != map[i] ){
            map[i] = digitalRead( i );
            Serial.println( rtc.now().timestamp( DateTime::TIMESTAMP_FULL ) );
            Serial.print( i );
            Serial.print( " -> " );
            Serial.println( map[i] );
            static int interations = 100; if( --interations <= 0 ) exit(0); 
        }
    }
#endif
}   
