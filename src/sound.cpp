#include <Arduino.h>
#include <TeensyThreads.h>
#include <TimeLib.h>
#include "sound.h"
#include "console.h"

Threads::Mutex _lock;
String GLOBAL_LOG;

char NOISE_BUF[64];
unsigned int runCount = 0;
unsigned int noiseDuration = 0;
unsigned int noiseStart = 0;
volatile state_t  GlobalState = PREAMBLE;

int gatePIN = 23;
void soundSetup() {
    pinMode(gatePIN, INPUT);
}

unsigned int MAX_PREAMBLE_SAMPLES = 500; 
unsigned int preambleSampleIndex = 0;
uint8_t preambleSamples[500];
unsigned int preambleDuration = 0;
unsigned int MAX_SAMPLES = 75;
unsigned int sampleIndex = 0;
uint8_t samples[75];

extern int StartTime;

state_t readPreambleSignal() {
    if  (preambleSampleIndex >= MAX_PREAMBLE_SAMPLES) {
        preambleSampleIndex = 0;
        int positiveSamples = 0;
        for (unsigned int i = 0; i < MAX_PREAMBLE_SAMPLES; i++) {
            if (preambleSamples[i] != 0) {
                positiveSamples++;
            }
        }

        if (positiveSamples > 50) {
            noiseDuration = 2;
            noiseStart = now();
            return MUSIC_DETECTED;
        }
        else {
            return PREAMBLE;
        }
    }
    unsigned int val = digitalRead(gatePIN);
    preambleSamples[preambleSampleIndex] = val;
    preambleSampleIndex++;
    return PREAMBLE;
}

char result_buf[256];
state_t readAudioSignal() {
    bool continueSampling = false;
    noiseDuration = now() - noiseStart;
    if  (sampleIndex >= MAX_SAMPLES) {
        sampleIndex = 0;
        for (unsigned int i = 0; i < MAX_SAMPLES; i++) {
            if (samples[i] != 0) {
                continueSampling = true;
                break;
            }
        }

        if (!continueSampling) {
            //noiseDuration -= 5;
            //OpenLog.printf("%02d:%02d:%02d Detected Song #%02u at %lu with duration %u\n",
            //        hour(), minute(), second(), runCount + 1, (now() - noiseDuration) - (StartTime + 2), noiseDuration);
            snprintf(result_buf, sizeof(result_buf), "%02d:%02d:%02d Detected Song #%02u at %u with duration %u\n<br>", 
                    hour(), minute(), second(), runCount + 1,  StartTime-noiseStart, noiseDuration);
            noiseDuration--;

            _lock.lock();
            GLOBAL_LOG += String(result_buf);
            _lock.unlock();

            if (noiseDuration > 30) {
                runCount++;
            }
            return PREAMBLE;
        }
    }
    unsigned int val = digitalReadFast(gatePIN);
    samples[sampleIndex] = val;
    sampleIndex++;
    return MUSIC_DETECTED;
}

void transitionState() {
    /* GLOBAL STATE TESTING
       if (GlobalState == PREAMBLE)
       GlobalState = MUSIC_DETECTED;
       else if (GlobalState == MUSIC_DETECTED)
       GlobalState = PREAMBLE;
       */

    switch (GlobalState) {
        case PREAMBLE:
            {
                GlobalState = readPreambleSignal();
                threads.delay(1);
                break;
            }
        case MUSIC_DETECTED:
            {
                GlobalState = readAudioSignal();
                threads.delay(20);
                break;
            }
        default: 
            break;
    }
}

void soundThread() {
    while (1) {
        transitionState();
        threads.yield();
        //threads.delay(333);
    }
}

String getLog() {
    String ret;
    Threads::Scope m(_lock);
    ret = GLOBAL_LOG;
    return ret;
}

