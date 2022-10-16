#include "IrSystem.h"

#define EXTERNAL_ARDUINO
//#define INTERNAL_ARDUINO

#define VALV_INTERNAL 8
#define VALV_FRONT 6 
#define VALV_BACK 5

RTC_DS3231 rtc;
IrSystem irSys;

#define GROUP_INTERNAL 0
#define GROUP_EXTERNAL 1

void setup(){
    Serial.begin( 9600 );
    while( !rtc.begin() );
    if( rtc.lostPower() ){
#ifdef TEST
        rtc.adjust( DateTime(2000, 1, 1, 8, 59, 55 ) );
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
    uint8_t tenSec = 1;
#else
    uint16_t fiveMin = 5 * 60;
    uint8_t tenSec = 10;  
#endif

    irSys.addValv( VALV_INTERNAL, GROUP_INTERNAL );
    irSys.addValv( VALV_FRONT, GROUP_EXTERNAL );
    irSys.addValv( VALV_BACK, GROUP_EXTERNAL );

    irSys.getValv( VALV_INTERNAL, GROUP_INTERNAL )->addConfig( Config( 9, 0, 0, fiveMin, false, tenSec, ENQUEUED ) );
    irSys.getValv( VALV_FRONT, GROUP_EXTERNAL )->addConfig( Config( 9, 0, 1, fiveMin, true, tenSec, ENQUEUED ) ); 
    irSys.getValv( VALV_BACK, GROUP_EXTERNAL )->addConfig( Config( 9, 0, 2, fiveMin, true, tenSec, ENQUEUED ) );


#if defined( EXTERNAL_ARDUINO )  
    irSys.deactivateGroup( GROUP_INTERNAL );
#endif
#if defined( INTERNAL_ARDUINO )
    irSys.deactivateGroup( GROUP_EXTERNAL );
#endif

}

void loop(){
    irSys.run( rtc.now() );
    
    static uint32_t lastMillis = millis();
    if( millis() - lastMillis >= 1000 ){
        digitalWrite( LED_BUILTIN, !digitalRead( LED_BUILTIN ) );
        lastMillis = millis();
    }

    static int map[14];
    if( ( map[0] == 0 ) ) { map[0] = 1; for( int i = 1; i <= 14; i++ ) map[i] = digitalRead( i ); }

    for( int i = 1; i < 13; i++ ){
        if( digitalRead( i ) != map[i] ){
            map[i] = digitalRead( i );
            Serial.print( "");
            Serial.print( "TestTime: " );           
            Serial.println( rtc.now().timestamp( DateTime::TIMESTAMP_TIME ) );
            Serial.print( i );
            ( map[i] ) ? Serial.println( " -> OFF " ) : Serial.println( " -> ON " );

            static int interations = 6; if( --interations <= 0 ) exit(0); 
        }
    }
}   
