#include "irrigation_system.h"

#define NUM_VALV 2
Valv arrValv[NUM_VALV];
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

    arrValv[0].begin( 7 );
    arrValv[1].begin( 8 );

    arrValv[0].addConfig( Config( 9, 0, 0, 1, 5, 1 ) );  
    arrValv[1].addConfig( Config( 9, 0, 1, 1, 5, 1 ) );  
}

void loop(){
    irSys.checkConfigs( &arrValv[0] );
    irSys.checkConfigs( &arrValv[1] );

    Serial.println( irSys.simultaneous->pin );
    //Serial.println( irSys.simultaneous->nextNode->pin );

#ifdef TEST
    exit(0); 
#endif
}   

/** Config METHODS */

Config::Config( uint8_t hour, uint8_t min, uint8_t sec, uint16_t minEvent, uint16_t maxEvent, uint8_t type ){
    this->sec = sec;
    this->min = min;
    this->hour = hour;
    this->minEvent = minEvent;
    this->maxEvent = maxEvent;
    this->type = type;
}

Config::Config( rawConfigData rawData ){
    this->sec =      ( uint8_t )  (   rawData & ( uint64_t )  0x3F );
    this->min =      ( uint8_t )  ( ( rawData & ( uint64_t )  0x3F <<  6 ) >>  6 );
    this->hour =     ( uint8_t )  ( ( rawData & ( uint64_t )  0x1F << 12 ) >> 12 );
    this->minEvent = ( uint16_t ) ( ( rawData & ( uint64_t ) 0xFFF << 17 ) >> 17 );
    this->maxEvent = ( uint16_t ) ( ( rawData & ( uint64_t ) 0xFFF << 29 ) >> 29 );
    this->type     = ( uint8_t )  ( ( rawData & ( uint64_t )   0x1 << 41 ) >> 41 );
}

rawConfigData Config::toRaw(){
    while( this->sec >= 60 ){
        this->sec -= 60;
        this->min++;
    }
    while( this->min >= 60 ){
        this->min -= 0;
        this->hour++;
    }   
    while( this->hour >= 24 ){ 
       this->hour -= 24; }

    if( this->minEvent > 0xFFF ) this->minEvent = 0xFFF;
    if( this->maxEvent > 0xFFF ) this->maxEvent = 0xFFF;

    return ( 
        ( ( uint64_t ) this->sec ) | 
        ( ( uint64_t ) this->min      <<  6 ) |
        ( ( uint64_t ) this->hour     << 12 ) |  
        ( ( uint64_t ) this->minEvent << 17 ) | 
        ( ( uint64_t ) this->maxEvent << 29 ) |
        ( ( uint64_t ) this->type     << 41 ) );
}

/** Valv METHODS */

void Valv::begin( uint8_t pin ){
    this->configsCount = 0;
    this->pin = pin;
    pinMode( this->pin, OUTPUT );
    digitalWrite( this->pin, LOW );
}

bool Valv::addConfig( Config newConfig ){
    if( this->configsCount < MAX_CONFIG_NUM ){
        this->config[ this->configsCount++ ] = newConfig.toRaw();
        return true;
    }
    return false;
}

uint8_t Valv::getPin(){
    return this->pin;
}
rawConfigData Valv::getConfig( uint8_t index ){
    return this->config[ index ];
}
rawConfigData* Valv::getConfigs(){
    return this->config;
}
uint8_t Valv::getCountConfig(){
    return this->configsCount;
}


/** IrSystem METHODS */

void IrSystem::add( uint8_t pin, uint16_t time, uint8_t type ){
    node** handler;
    if( type == SIMULTANEOUS ){
        handler = &this->simultaneous;
    }else{
        handler = &this->enqueued;
    }
    
    node* newNode = new node;
    newNode->nextNode = NULL;
    newNode->pin = pin;
    newNode->time = time;

    while ( *handler != NULL ){
        handler = &( *handler )->nextNode;
    }
    ( *handler ) = newNode; 
}

void IrSystem::checkConfigs( Valv* valv ){
    static Config readingConfig;
    static DateTime now = rtc.now();

    for( int i = 0; i < valv->getCountConfig(); i++ ){
        readingConfig = Config( valv->getConfig( i ) );
        if( ( readingConfig.hour == now.hour() ) &&
            ( readingConfig.min == now.minute() ) &&
            ( readingConfig.sec == now.second() ) ){
            Serial.println( readingConfig.sec );
            this->add( valv->getPin(), readingConfig.maxEvent, readingConfig.type );    
        }

    }
}