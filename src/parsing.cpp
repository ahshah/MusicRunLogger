#include <Arduino.h>
#include "console.h"

void parseHTTPRequest(char* buffer, char** method, char** resource, char** protocol) {
    char* ptr;
    char* ptrptr;
    char* method_resource_protocol;

    //Console.printf("RX buf: %s\r\n", buffer);

    method_resource_protocol = strtok_r(buffer,"\r\n", &ptr);
   // Console.printf("RX buf method: %s \r\n", method);

    *method = strtok_r(method_resource_protocol, " ", &ptrptr);
  //  Console.printf("RX buf method: %s \r\n", *method);

    *resource = strtok_r(NULL, " ", &ptrptr);
 //   Console.printf("RX buf resource: %s \r\n", *resource);

    *protocol = strtok_r(NULL, "\r\n", &ptrptr);
//    Console.printf("RX buf protocol: %s \r\n", *protocol);

//    Console.printf("Got Method: %s\r\nGot Resource: %s\r\nGot Protocol: %s\r\n", 
//            *method, *resource, *protocol);
    Console.printf("Got Resource: %s\r\n", *resource);

}
