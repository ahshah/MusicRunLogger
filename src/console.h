#ifndef CONSOLE_H
#define CONSOLE_H

#include <Arduino.h>
#include <TeensyThreads.h>

extern Threads::Grab<decltype(Serial)> GlobalSerial;
#define Serial ThreadClone(GlobalSerial)
#define Console ThreadClone(GlobalSerial)

#endif
