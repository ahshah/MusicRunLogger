#include <Arduino.h>
#include <TeensyThreads.h>

int MULTITHREADED_ENABLED = 0;
Threads::Grab<decltype(Serial)> GlobalSerial(Serial);
