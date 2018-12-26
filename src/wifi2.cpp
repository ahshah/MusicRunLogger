#include <ESP8266.h>
#include <TeensyThreads.h>
#include <TimeLib.h>
#include "sound.h"
#include "console.h"
ESP8266 wifi(Serial2, 115200);

void parseHTTPRequest(char* buffer, char**, char**, char** );
void handleGET(uint8_t mux_id, const String& resource);
void handlePOST(uint8_t mux_id, const String& resource);
int CH_PD_PIN = 0x06;
int CTS_PIN = 23;

void resetESP(void) {
    pinMode(CH_PD_PIN, OUTPUT);
    pinMode(CTS_PIN, OUTPUT);

    digitalWriteFast(CTS_PIN, LOW);
    delay(500);

    digitalWriteFast(CH_PD_PIN, LOW);
    delay(100);
    digitalWriteFast(CH_PD_PIN, HIGH);
    delay(500);
}

void setupESP(void)
{
//    Serial.begin(115200);
//    while(!Serial);

    Console.print("setup begin\r\n");
    resetESP();
    wifi.restart();

    Console.print("FW Version:");
    Console.println(wifi.getVersion().c_str());

    if (wifi.setOprToStationSoftAP()) {
        Console.print("to station + softap ok\r\n");
    } else {
        Console.print("to station + softap err\r\n");
    }

    String retstr = wifi.runCommand("AT+CWSAP?");
    Console.printf("Check config: %s\r\n", retstr.c_str());
//    Console.printf("Check DHCP: %s\r\n", wifi.runCommand("AT+CWDHCP?").c_str());

    if(wifi.setSoftAPParam("JackInTheBox", "jacksonhoward", 4, 3)) {
        Console.print("Create AP success\r\n");
    } 
    else {
        Console.print("Create AP failure\r\n");
    }
    wifi.restart();
    delay(1000);
    retstr = wifi.runCommand("AT+CWSAP?");
    Console.printf("Check config2: %s\r\n", retstr.c_str());
    //Console.printf("Check config2: %s\r\n", wifi.runCommand("AT+CWSAP?").c_str());

    if (wifi.enableMUX()) {
        Console.print("multiple ok\r\n");
    } else {
        Console.print("multiple err\r\n");
    }

    if (wifi.startTCPServer(80)) {
        Console.print("start tcp server ok\r\n");
    } else {
        Console.print("start tcp server err\r\n");
    }

    if (wifi.setTCPServerTimeout(10)) { 
        Console.print("set tcp server timout 10 seconds\r\n");
    } else {
        Console.print("set tcp server timout err\r\n");
    }

    delay(1000);
    String ip = wifi.getAccessPointIP();
    Console.printf("setup end IP: %s\r\n", ip.c_str());
    wifi.SpecialBaud();

    bool ret = Serial2.attachCts(23);
   Console.printf("CTS setup? %d\r\n", ret);
    ret = Serial2.attachRts(18);
   Console.printf("RTS setup? %d\r\n", ret);

}

void ESPReadThread() {
    while(1) {
        wifi.super_recv();
//        threads.delay(5);
        threads.yield();
//    Console.printf("setup end IP: %s\r\n", wifi.getLocalIP().c_str());
    }
}

void ESPWriteThread() {
    while(1) {
//        wifi.super_tx();
        wifi.stateful_tx();
        threads.delay(5);
//        threads.yield();
//    Console.printf("setup end IP: %s\r\n", wifi.getLocalIP().c_str());
    }
}

void ESPProcessThread(void)
{
    recv_msg_t msg;
    while(1) {
        if (wifi.super_recv_mux_done(&msg)) {
            //Console.printf("Super received messaged\r\n");
            char* method;
            char* resource;
            char* protocol;
            msg.data[msg.len] = '\0';
            parseHTTPRequest(msg.data, &method, &resource, &protocol);
            if ( 0 == strcmp(method, "GET")) {
               handleGET(msg.mux, String(resource));
            }
            else if ( 0 == strcmp(method, "POST")) {
               handlePOST(msg.mux, String(resource));
            }
            else {
               Console.printf("Unknown method");
            }
            //wifi.releaseTCP(msg.mux);
        }
        threads.delay(33);
        //threads.yield();
        //Console.printf("setup end IP: %s\r\n", wifi.getLocalIP().c_str());
    }
}


char TIME_BUF[32] = {0};
extern int StartTime;

void handleGET(uint8_t mux_id, const String& resource) {
//    Console.printf("replying to connection mux: %d\r\n", mux_id);
    String resp = "HTTP/1.0 200 OK\r\nServer:TeensyWeb\r\nConnection:Closed\r\n" ;
    String body = "";
    char buf[16];

    snprintf(TIME_BUF, 100, "%02d:%02d:%02d Up:%lu", hour() > 12 ? hour()-12:hour(), minute(), second(), now() - StartTime);
    if (resource == "/" || resource == "/index.html" || resource == "index.htm" || resource == "favicon.ico") {
        //Console.printf("Handling index.html\r\n");
        body += "<html>Hello world! <b>" + String(TIME_BUF) + "</b></html>\r\n";
    }
    else if (resource == "/long") {
        body += "<html><p>Four score and seven years ago our fathers brought forth,"
             "upon this continent, a new nation, conceived in liberty, and"
             "dedicated to the proposition that all men are created equal."
             "Now we are engaged in a great civil war, testing whether that"
             "nation, or any nation so conceived, and so dedicated, can long"
             "endure. We are met on a great battle field of that war. We"
             "come to dedicate a portion of it, as a final resting place for"
             "those who died here, that the nation might live. This we may,"
             "in all propriety do. But, in a larger sense, we can not dedicate"
             "we can not consecrate we can not hallow, this ground The brave"
             "men, living and dead, who struggled here, have hallowed it,"
             "far above our poor power to add or detract. The world will little"
             "note, nor long remember what we say here; while it can never"
             "forget what they did here.It is rather for us, the living, we"
             "here be dedicated to the great task remaining before us that,"
             "from these honored dead we take increased devotion to that cause"
             "for which they here, gave the last full measure of devotion"
             "that we here highly resolve these dead shall not have died"
             "in vain; that the nation, shall have a new birth of"
             "freedom, and that government of the people, by the"
             "people, for the people, shall not perish from the"
             "earth.</p></html>\r\n";
    }
    else if(resource == "/speech0") {
        body += "<html><p>Four score and seven years ago our fathers brought forth,"
             "upon this continent, a new nation, conceived in liberty, and"
             "dedicated to the proposition that all men are created equal.";

    }
    else if(resource == "/speech1") {

        body += "Now we are engaged in a great civil war, testing whether that"
             "nation, or any nation so conceived, and so dedicated, can long"
             "endure. We are met on a great battle field of that war. We"
             "come to dedicate a portion of it, as a final resting place for"
             "those who died here, that the nation might live. This we may,";
    }
    else if(resource == "/speech2") {
            body += "in all propriety do. But, in a larger sense, we can not dedicate"
             "we can not consecrate we can not hallow, this ground The brave"
             "men, living and dead, who struggled here, have hallowed it,"
             "far above our poor power to add or detract. The world will little"
             "note, nor long remember what we say here; while it can never";
    }
    else if(resource == "/speech3") {
        body += "forget what they did here.It is rather for us, the living, we"
             "here be dedicated to the great task remaining before us that,"
             "from these honored dead we take increased devotion to that cause"
             "for which they here, gave the last full measure of devotion"
             "that we here highly resolve these dead shall not have died"
             "in vain; that the nation, shall have a new birth of"
             "freedom, and that government of the people, by the"
             "people, for the people, shall not perish from the"
             "earth.</p></html>\r\n";
    }


    else if (strstr(resource.c_str(), "/AT") == resource.c_str()) {
        Console.printf("Running AT command...\r\n");
        String ret = wifi.runCommand(String(resource.c_str()+1));
        body += "<html> <p>" + ret  + "</p> </html>";
    }
    else if (strstr(resource.c_str(), "/log") == resource.c_str()) {
        String s = getLog();
        body += "<html> <p>" + s + "</p> </html>";
    }
    else if (strstr(resource.c_str(), "/reset") == resource.c_str()) {
        wifi.softReset();
        body += "<html> OK </html>";
    }


    snprintf(buf, 16, "%u", body.length());
    resp += "Content-Length:" + String(buf) + "\r\n";
    resp +="\r\n";
    resp += body;

    //bool ret = wifi.send(mux_id, (uint8_t*)resp.c_str(), resp.length());
    //Console.printf("Queuing message for mux {%d}... ", mux_id);
    bool ret = wifi.queue(mux_id, (uint8_t*)resp.c_str(), resp.length());
    if (!ret) {
        Console.printf("couldn't queue..\r\n");
    }
    //Console.printf("sending.... %d", mux_id);
#if 0
    if(wifi.send(mux_id, (uint8_t*)resp.c_str(), resp.length())) {
     //   Console.printf("SUCCESS\r\n");
    }
    else {
      //  Console.printf("FAILURE\r\n");
    }
//    Console.printf("done\r\n");
#endif
}

void handlePOST(uint8_t mux_id, const String& resource) {
    char* loc = strstr(resource.c_str(), "/SetTime?");
    int asciiLength = resource.length()-strlen("/SetTime?");
    time_t currentTime;
    char numBuf[16];
    if (loc == resource.c_str() && asciiLength <=10) {
        strncpy(numBuf, resource.c_str()+strlen("/SetTime?"), 10);
        numBuf[asciiLength] = '\0';
        currentTime = (time_t)atoi(numBuf);
        Teensy3Clock.set(currentTime); // set the RTC
        setTime(currentTime);
    }
}
