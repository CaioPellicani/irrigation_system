#include <AUnit.h>
#include "IrSystem.h"

using namespace aunit;

test(testConfigTranslation) {
    int secHigh = 5 * 60;
    Config config = Config( 9, 0, 0, secHigh, false, 10, ENQUEUED );
    Config raw_config = Config(config.toRaw());
    assertEqual(config.toRaw(), raw_config.toRaw());
    assertEqual(config.secHIGH, secHigh);
    assertEqual(raw_config.secHIGH, secHigh);
}


class ValvTest: public TestOnce {
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

testF(ValvTest, testRTC) {


}