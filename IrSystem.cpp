#include "IrSystem.h"
/** Config METHODS */

Config::Config( uint8_t hour, uint8_t min, uint8_t sec, uint16_t secHIGH, bool useMonthlyPercent, uint8_t pause, uint8_t type ){
    this->sec = sec;
    this->min = min;
    this->hour = hour;
    this->pause = pause;
    this->secHIGH = secHIGH;
    this->useMonthlyPercent = useMonthlyPercent;
    this->type = type;
}

#define BIT_OP( type, mask, shift ) ( type )( ( rawData & ( uint64_t ) mask << shift ) >> shift )
Config::Config( rawConfigData rawData ){
    this->sec =               BIT_OP(  uint8_t,  0x3F,  0 );
    this->min =               BIT_OP(  uint8_t,  0x3F,  6 );
    this->hour =              BIT_OP(  uint8_t,  0x1F, 12 );
    this->pause =             BIT_OP(  uint8_t,  0x3F, 17 );
    this->secHIGH =           BIT_OP( uint16_t, 0xFFF, 23 );
    this->useMonthlyPercent = BIT_OP(  uint8_t,   0x1, 35 );
    this->type =              BIT_OP(  uint8_t,   0x1, 36 );
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
uint8_t Valv::getCountConfig(){
    return this->configsCount;
}


/** IrSystem METHODS */

IrSystem::IrSystem(){
    this->monthlyPercent( 100, 100, 100, 100, 100, 100,
                          100, 100, 100, 100, 100, 100 );
}
void IrSystem::monthlyPercent( uint8_t jan, uint8_t feb, uint8_t mar, uint8_t apr, 
                                  uint8_t may, uint8_t jun, uint8_t jul, uint8_t ago, 
                                  uint8_t sep, uint8_t out, uint8_t nov, uint8_t dez ){
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

void IrSystem::add( uint8_t pin, uint16_t time, uint8_t pause, uint8_t type ){
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
            ( *eNode )->time--; 
        }else{
            ( *eNode )->pause--;
        }
    }
    if( ( *eNode )->time > 0 ){
        digitalWrite( ( *eNode )->pin, HIGH );
    }else{
        digitalWrite( ( *eNode )->pin, LOW );
        if( ( *eNode )->pause == 0 ){
            this->remove( eNode );            
        }      
    }
}

bool IrSystem::isExecuting( uint8_t pin ){
    node** headList[2] = { &this->simultaneous, &this->enqueued };
    node** handler;
    for( int i = 0; i < 2; i++ ){
        handler = headList[i];
        while( ( *handler ) != NULL ){
            if( ( *handler )->pin == pin ){
                return true;
            }
            handler = &( *handler )->nextNode;
        }
    }
    return false;
}

void IrSystem::addValv( uint8_t pin ){
    vNode** handler = &valvs;    
    while ( *handler != NULL ){
        handler = &( *handler )->nextNode;
    }
    ( *handler ) = new vNode;
    ( *handler )->nextNode = NULL;
    ( *handler )->valv.begin( pin ); 
}

Valv* IrSystem::getValv( uint8_t pin ){
    vNode** handler = &valvs; 
    while( ( ( *handler ) != NULL ) && ( ( *handler )->valv.getPin() != pin ) ){
        handler = &( *handler )->nextNode;
    }
    return &( *handler )->valv;
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
                ( !this->isExecuting( valv->getPin() ) )
            ){
                this->add(  valv->getPin(), 
                            readingConfig.secHIGH, 
                            readingConfig.pause, 
                            readingConfig.type );    
            }
        }
        handler = &( *handler )->nextNode;
    }
}