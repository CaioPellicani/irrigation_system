#include "IrSystem.h"

/** Config METHODS */

const bool high = LOW;
#define RELAY_ON high
#define RELAY_OFF !high

Config::Config(){}
Config::Config( uint8_t hour, uint8_t min, uint8_t sec, uint16_t secHIGH, bool useMonthlyPercent, uint8_t pause, uint8_t type ){
    this->sec = sec;
    this->min = min;
    this->hour = hour;
    this->pause = pause;
    this->secHIGH = secHIGH;
    this->useMonthlyPercent = useMonthlyPercent;
    this->type = type;
}

#define BIT_OP( data, type, mask, shift ) ( type )( ( data & ( uint64_t ) mask << shift ) >> shift )
Config::Config( rawConfigData rawData ){
    this->sec =               BIT_OP( rawData,  uint8_t,  0x3F,  0 );
    this->min =               BIT_OP( rawData,  uint8_t,  0x3F,  6 );
    this->hour =              BIT_OP( rawData,  uint8_t,  0x1F, 12 );
    this->pause =             BIT_OP( rawData,  uint8_t,  0x3F, 17 );
    this->secHIGH =           BIT_OP( rawData, uint16_t, 0xFFF, 23 );
    this->useMonthlyPercent = BIT_OP( rawData,  uint8_t,   0x1, 35 );
    this->type =              BIT_OP( rawData,  uint8_t,   0x1, 36 );
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

    if( this->secHIGH > 0xFFF ) this->secHIGH = 0xFFF;

    return ( 
        ( ( uint64_t ) this->sec               <<  0 ) | 
        ( ( uint64_t ) this->min               <<  6 ) |
        ( ( uint64_t ) this->hour              << 12 ) | 
        ( ( uint64_t ) this->pause             << 17 ) | 
        ( ( uint64_t ) this->secHIGH           << 23 ) |
        ( ( uint64_t ) this->useMonthlyPercent << 35 ) |
        ( ( uint64_t ) this->type              << 36 ) );
}

/** Valv METHODS */
Valv::Valv(){}
void Valv::begin( uint8_t pin, uint8_t group ){
    this->configsCount = 0;
    this->pin = pin;
    this->group = group;
    pinMode( this->pin, OUTPUT );
    digitalWrite( this->pin, RELAY_OFF );
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

uint8_t Valv::getGroup(){
    return this->group;
}
rawConfigData Valv::getConfig( uint8_t index ){
    return this->config[ index ];
}
uint8_t Valv::getCountConfig(){
    return this->configsCount;
}

/** IrSystem METHODS */

IrSystem::IrSystem(){
    this->groupsState = 0xFF;
    //Serial.println( this->groupsState);
    this->monthlyPercent( 100, 100, 100, 100, 100, 100,
                          100, 100, 100, 100, 100, 100 );
}
void IrSystem::monthlyPercent( float jan, float feb, float mar, float apr, 
                               float may, float jun, float jul, float ago, 
                               float sep, float out, float nov, float dez ){
    this->arrMonthlyPercent[ 0] = jan; 
    this->arrMonthlyPercent[ 1] = feb; 
    this->arrMonthlyPercent[ 2] = mar; 
    this->arrMonthlyPercent[ 3] = apr; 
    this->arrMonthlyPercent[ 4] = may; 
    this->arrMonthlyPercent[ 5] = jun; 
    this->arrMonthlyPercent[ 6] = jul; 
    this->arrMonthlyPercent[ 7] = ago; 
    this->arrMonthlyPercent[ 8] = sep; 
    this->arrMonthlyPercent[ 9] = out; 
    this->arrMonthlyPercent[10] = nov; 
    this->arrMonthlyPercent[11] = dez; 
}

void IrSystem::deactivateGroup( uint8_t group ){
    if( this->groupsState & 1 << ( group ) ){
        this->groupsState ^= 1 << ( group );
    }
}

void IrSystem::activateGroup( uint8_t group ){
    this->groupsState |= 1 << ( group );
}

void IrSystem::add( uint8_t pin, uint8_t group, uint16_t time, uint8_t pause, uint8_t type ){
    node** handler;
    if( type == SIMULTANEOUS ){
        handler = &this->simultaneous;
    }else{
        handler = &this->enqueued;
    }

    while ( *handler != NULL ){
        handler = &( *handler )->nextNode;
    }
    ( *handler ) = new node;
    ( *handler )->nextNode = NULL;
    ( *handler )->pin = pin;
    ( *handler )->group = group;
    ( *handler )->time = time;
    ( *handler )->pause = pause;
    ( *handler )->lastMillis = 0;
}

void IrSystem::remove( node** dNode ){
    node* deadNode = *dNode;
    *dNode = deadNode->nextNode;
    delete deadNode;
}

void IrSystem::run( DateTime now ){
    this->checkConfigs( now );

    if( this->enqueued != NULL ){
        this->execute( &this->enqueued );
    }

    node** handler = &this->simultaneous;
    while ( ( handler != NULL ) && ( *handler != NULL ) ){
        this->execute( handler );
        handler = &( *handler )->nextNode;
    }
}

void IrSystem::execute( node** eNode ){
    if( ( *eNode )->lastMillis == 0 ){
        ( *eNode )->lastMillis = millis();
    }
    if( millis() - ( *eNode )->lastMillis >= 1000 ){
        ( *eNode )->lastMillis = millis();
        if( ( *eNode )->time > 0 ){
            Serial.print(( *eNode )->pin);
            Serial.print(" - time - ");
            Serial.println(( *eNode )->time);
            ( *eNode )->time--;
        }else{
            Serial.print(( *eNode )->pin);
            Serial.print(" - pause - ");
            Serial.println(( *eNode )->pause);
            ( *eNode )->pause--;
        }
    }
    if( ( *eNode )->time > 0 ){
        if( this->groupsState & 1 << ( ( *eNode )->group ) ){     
            digitalWrite( ( *eNode )->pin, RELAY_ON );
        }
    }else{
        digitalWrite( ( *eNode )->pin, RELAY_OFF );    
        if( ( *eNode )->pause == 0 ){
            this->remove( eNode );                
        }      
    }
}

bool IrSystem::isExecuting( uint8_t pin, uint8_t group ){
    node** headList[2] = { &this->simultaneous, &this->enqueued };
    node** handler;
    for( int i = 0; i < 2; i++ ){
        handler = headList[i];
        while( ( *handler ) != NULL ){
            if( ( ( *handler )->pin == pin ) && ( ( *handler)->group == group ) ){
                return true;
            }
            handler = &( *handler )->nextNode;
        }
    }
    return false;
}

Valv* IrSystem::addValv( uint8_t pin, uint8_t group ){
    vNode** handler = &valvs;    
    while ( *handler != NULL ){
        handler = &( *handler )->nextNode;
    }
    ( *handler ) = new vNode;

    if( ( *handler ) == NULL ) return NULL;

    ( *handler )->nextNode = NULL;
    ( *handler )->valv.begin( pin, group ); 
    return &( *handler )->valv;
}

Valv* IrSystem::getValv( uint8_t pin, uint8_t group ){
    vNode** handler = &valvs; 
    Valv* valv;
    while( ( *handler ) != NULL ){
        valv = &( *handler )->valv;
        if( ( valv->getPin() == pin ) && ( valv->getGroup() == group ) ){
            return valv;
        }
        handler = &( *handler )->nextNode;
    }
    return NULL;
}

void IrSystem::checkConfigs( DateTime now ){

    Config readingConfig;
    vNode** handler = &valvs;
    while( ( *handler ) != NULL ){
        Valv* valv = &( *handler )->valv;
        for( int i = 0; i < valv->getCountConfig(); i++ ){
            readingConfig = Config( valv->getConfig( i ) );
            if( ( readingConfig.sec == now.second() ) &&
                ( readingConfig.min == now.minute() ) && 
                ( readingConfig.hour == now.hour() ) &&
                ( !this->isExecuting( valv->getPin(), valv->getGroup() ) )
            ){
                this->add(  valv->getPin(), 
                            valv->getGroup(),
                            this->calculateTime( readingConfig.secHIGH, 
                                                 readingConfig.useMonthlyPercent, 
                                                 now ), 
                            readingConfig.pause, 
                            readingConfig.type );    
            }
        }
        handler = &( *handler )->nextNode;
    }
}

uint16_t IrSystem::calculateTime( uint16_t secHIGH, bool useMonthlyPercent, DateTime now ){
    if( useMonthlyPercent ){
        static uint8_t monthsDays[12] = { 31, 28, 31, 30, 31, 30, 
                                          31, 31, 30, 31, 30, 31 };
        uint8_t month = now.month() - 1;
        uint8_t nextMonth = ( now.month() == 12 ) ? 0 : month + 1;

        uint8_t lastDay = monthsDays[ month ];
        uint32_t min = ( uint32_t ) this->arrMonthlyPercent[ month ] * 1000;
        uint32_t max = ( uint32_t ) this->arrMonthlyPercent[ nextMonth ] * 1000;

        uint32_t percent = map( now.day(), 1, lastDay, min, max );
        int result = ( secHIGH * percent ) / 100000;
        return result;
    }else{
        return secHIGH;
    }
}