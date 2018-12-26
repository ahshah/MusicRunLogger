#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TeensyThreads.h>
#include <TimeLib.h>
#include "console.h"
#include "wifi2.h"
#include "rtc.h"
#include "view.h"
#include "sound.h"

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-
//Connect RXI of OpenLog to pin 5 on Arduino
//SoftwareSerial OpenLog(0, 1); // 0 = Soft RX pin (not used), 5 = Soft TX pin
//5 can be changed to any pin. See limitation section on https://www.arduino.cc/en/Reference/SoftwareSerial
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-

int statLED = 13;



float dummyVoltage = 3.50; //This just shows to to write variables to OpenLog
/*
void openLogSetup() {
    StartTime = now();
    OpenLog.begin(9600); //Open software serial port at 9600bps
    //Write something to OpenLog
    OpenLog.printf("Jaxy in the box start up @ %d.%d.%d %02d:%02d:%02d\n",
            year(), month(), day(), hour(), minute(), second());
}
*/

void setupSerial() {
    time_t start = millis();
    pinMode(statLED, OUTPUT);
    Console.begin(115200);
    CORE_PIN0_CONFIG = 0;
    CORE_PIN1_CONFIG = 0;

    // Enable new pins
    CORE_PIN21_CONFIG = PORT_PCR_PE | PORT_PCR_PS | PORT_PCR_PFE | PORT_PCR_MUX(3);
    CORE_PIN5_CONFIG = PORT_PCR_DSE | PORT_PCR_SRE | PORT_PCR_MUX(3);


    while (!Serial) {
        delay(500);
        if (millis() - start > 3000) {
            break;
       }
    }
}

volatile int count = 0;
void loop() {
    //    delay(5000);
    //    Console.println(count);
}

void thread_func(int data){
    int i =0;
    while(1) { 
        if ( i++ % 2 == 0) {
            digitalWriteFast(13, HIGH);
        }
        else {
            digitalWriteFast(13, LOW);
        }
        threads.delay(1000);
    }
}
void setup() {
    setupRTC();
    setupSerial();
    //    openLogSetup();
    setupESP();
    teensyViewSetup();
    //soundSetup();
    delay(500);

    //delay(1000);


    //    Console.println("Threading wifi..");
    //    threads.addThread(thread_func, 1);
    //    threads.setDefaultTimeSlice(1);
    threads.setMicroTimer(100);
    threads.setSliceMicros(5);
    threads.addThread(thread_func);

    int id = threads.addThread(ESPProcessThread, 0,9096);
    threads.setTimeSlice(id, 5);

    int t_id = threads.addThread(ESPReadThread, 0, 9096);
    threads.setTimeSlice(t_id, 50);

    int w_id = threads.addThread(ESPWriteThread, 0, 1724);
    threads.setTimeSlice(w_id, 5);

    int v_id = threads.addThread(teensyViewThread);
    threads.setTimeSlice(v_id, 5);

//    int s_id = threads.addThread(soundThread, 0, 4096);
//    threads.setTimeSlice(s_id, 100);
}
