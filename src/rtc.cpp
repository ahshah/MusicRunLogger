#include <Arduino.h>
#include <TimeLib.h>
time_t StartTime = 0;
time_t getTeensy3Time()
{
    return Teensy3Clock.get();
}

void setupRTC() {
    // set the Time library to use Teensy 3.0's RTC to keep time
    setSyncProvider(getTeensy3Time);
    StartTime = now();
}


int NOW() {
    return now() - 14400; //EST
}
