#include "IrSystem.h"
#include <avr/wdt.h>

//#define TEST

//#define EXTERNAL_ARDUINO
#define INTERNAL_ARDUINO

#define VALV_INTERNAL 8
#define VALV_FRONT 3 
#define VALV_BACK 4
#define VALV_GRASS 5
#define VALV_WILDCARD 6

RTC_DS3231 rtc;
IrSystem irSys;

#define GROUP_INTERNAL 0
#define GROUP_EXTERNAL 1

void setup(){
    wdt_enable(WDTO_2S);
    Serial.begin( 9600 );
    while( !rtc.begin() );

#ifdef TEST
    Serial.println("IN TESTING MODE");
    rtc.adjust( DateTime(2000, 7, 1, 8, 59, 55 ) );
#else
    //if( rtc.lostPower() ){
        rtc.adjust( DateTime( F( __DATE__ ), F( __TIME__ ) ) );
        Serial.println( "Adjusting RTC" );
    //}
#endif
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
    irSys.addValv( VALV_GRASS, GROUP_EXTERNAL );
    irSys.addValv( VALV_WILDCARD, GROUP_EXTERNAL );

    irSys.getValv( VALV_INTERNAL, GROUP_INTERNAL )->addConfig( Config( 9, 0, 0, fiveMin, false, tenSec, ENQUEUED ) );
    irSys.getValv( VALV_FRONT, GROUP_EXTERNAL )->addConfig( Config( 9, 0, 1, fiveMin, true, tenSec, ENQUEUED ) ); 
    irSys.getValv( VALV_BACK, GROUP_EXTERNAL )->addConfig( Config( 9, 0, 2, fiveMin, true, tenSec, ENQUEUED ) );
    irSys.getValv( VALV_GRASS, GROUP_EXTERNAL )->addConfig( Config( 9, 0, 3, fiveMin, true, tenSec, ENQUEUED ) );
    irSys.getValv( VALV_WILDCARD, GROUP_EXTERNAL )->addConfig( Config( 9, 0, 4, fiveMin, true, tenSec, ENQUEUED ) );

#if defined( EXTERNAL_ARDUINO )  
    irSys.deactivateGroup( GROUP_INTERNAL );
    Serial.println("EXTERNAL_ARDUINO");
#endif
#if defined( INTERNAL_ARDUINO )
    irSys.deactivateGroup( GROUP_EXTERNAL );
    Serial.println("INTERNAL_ARDUINO");
#endif

}

void loop(){
    wdt_reset();
    irSys.run( rtc.now() );
    static int map[14];
   
    static uint32_t lastMillis = millis();
    if( millis() - lastMillis >= 1000 ){
        digitalWrite( LED_BUILTIN, !digitalRead( LED_BUILTIN ) );
        lastMillis = millis();
        Serial.println( rtc.now().timestamp( DateTime::TIMESTAMP_TIME ) );
    }
}   
