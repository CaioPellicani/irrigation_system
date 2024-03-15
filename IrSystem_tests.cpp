#include <AUnit.h>
#include "IrSystem.h"

using namespace aunit;

int secHigh = 5 * 60;
test(testConfigTranslation) {
    Config config = Config( 9, 0, 0, secHigh, false, 10, ENQUEUED );
    Config raw_config = Config(config.toRaw());
    assertEqual(config.toRaw(), raw_config.toRaw());
    assertEqual(config.secHIGH, secHigh);
    assertEqual(raw_config.secHIGH, secHigh);
}


test(valvTest){
    Valv valv = Valv();
    int pin = 4;
    int group = 1;
    Config config0 = Config( 9, 0, 0, secHigh, false, 10, ENQUEUED );
    Config config1 = Config( 10, 5, 6, secHigh, false, 10, ENQUEUED );

    valv.begin(pin, group);
    valv.addConfig(config0);

    assertEqual(valv.getPin(), pin);
    assertEqual(valv.getGroup(), group);
    assertEqual(valv.getCountConfig(), 1);
    assertEqual(valv.getConfig(0), config0.toRaw());

    valv.addConfig(config1);
    assertEqual(valv.getCountConfig(), 2);
    assertEqual(valv.getConfig(1), config1.toRaw());

    assertNotEqual(valv.getConfig(0), config1.toRaw());

}

class IRSystemTest: public TestOnce {
    protected:
    RTC_DS3231 rtc;
    DateTime beginTime;
    IrSystem irSys;

    void setup() override {
        TestOnce::setup();

        while(!rtc.begin());
        beginTime = DateTime( 2000, 1, 1, 0, 0, 5 );
        rtc.adjust(beginTime);
    }
};