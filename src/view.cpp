#include <TeensyView.h>
#include <TeensyThreads.h>
#include <TimeLib.h>
#include "rtc.h"
#include "sound.h"

///////////////////////////////////
// TeensyView Object Declaration //
// Non standard, broken view     //
#if 1
#define PIN_RESET 15
#define PIN_DC    5
#define PIN_CS    20
#define PIN_SCK   13
//#define PIN_MOSI  7
#define PIN_MOSI  11
#else
//Standard
#define PIN_RESET 15
#define PIN_DC    5
#define PIN_CS    10
#define PIN_SCK   13
#define PIN_MOSI  11
#endif
TeensyView oled(PIN_RESET, PIN_DC, PIN_CS, PIN_SCK, PIN_MOSI);

char DISPLAY_TIME_BUF[32];
char DISPLAY_EVNT_BUF[32];
char DISPLAY_RUNS_BUF[32];
extern unsigned int noiseDuration;
extern volatile state_t  GlobalState;
extern unsigned int runCount;
extern int recv_state;
extern int tx_state;
extern int rx_iter;
extern int tx_iter;


void printCountDown(String countDown, int font)
{
    volatile int middleX = oled.getLCDWidth() / 2;
    oled.clear(PAGE);
    oled.clear(PAGE);
    oled.setCursor(middleX - (oled.getFontWidth() * (countDown.length() / 2)),
            0);
    oled.print(countDown);
    oled.display();
    oled.clear(PAGE);
}

void teensyViewSetup()
{
    oled.begin();    // Initialize the OLED
    oled.clear(ALL); // Clear the display's internal memory
    oled.clear(PAGE); // Clear the buffer.
    //oled.display();  // Display what's in the buffer (splashscreen)
    for(int i =0; i < 15; i++) {
        snprintf(DISPLAY_TIME_BUF, 64, "Start %u", 15-i);
        printCountDown(DISPLAY_TIME_BUF, 1);
        delay(10);     // Delay 10 ms
    }
    //oled.clear(PAGE); // Clear the buffer.
    //randomSeed(analogRead(A0) + analogRead(A1));
}

// Center and print a small title
// This function is quick and dirty. Only works for titles one
// line long.
void printTitle(const String& title, const String& state, const String& run, const String& duration)
{
    unsigned int middleX = 1;
    oled.clear(PAGE);
    oled.setFontType(0);
    // Try to set the cursor in the middle of the screen
    // Print the title:
    oled.setCursor(0, 0);
    oled.print(title);

    // Print the duration:
    oled.setCursor(0, 16);
    oled.print(run);

    // Print the duration:
    oled.setCursor(0, 24);
    oled.print(duration);

    oled.display();
    oled.clear(PAGE);
}

const char* state_to_str[6] = {"NEW_CMD",
"STATUS",
"IPD_STATUS",
"IPD_MUX",
"IPD_LENGTH",
"IPD_FRAME"};

const char* tx_state_to_str[5] = {
"READY",
"TRANSFER",
"XFER_RES",
"XFER_COMP",
"TRANSMIT"
};


char chug[16] = {
'-',
'-',
'\\',
'\\',
'|',
'|',
'/',
'/',
'-',
'-',
'\\',
'\\',
'|',
'|',
'/',
'/',
};



void teensyViewThread()
{
    while (1) {
        snprintf(DISPLAY_TIME_BUF, 32, "%02d/%02d/%02d %02d:%02d:%0d", 
                day(), month(), year(), (hour() > 12 ? hour()-12:hour()), minute(), second());
        snprintf(DISPLAY_EVNT_BUF, 32, "Wifi TX %c %d %s", chug[tx_iter], tx_state, tx_state_to_str[tx_state]);
        snprintf(DISPLAY_RUNS_BUF, 32, "Wifi RX %c %s", chug[rx_iter], state_to_str[recv_state]);
        printTitle(DISPLAY_TIME_BUF, "PREAMBLE", DISPLAY_RUNS_BUF, DISPLAY_EVNT_BUF);

        threads.delay(33);
        threads.yield();
    }
}
