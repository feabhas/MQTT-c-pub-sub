//
//  main.c
//  mqtt_publisher
//
//  Created by Niall Cooling on 23/12/2011.
//  Copyright 2012 Feabhas Limited. All rights reserved.
//  
/*

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

// Current Assumes
// QoS 0
// All messages < 127 bytes
//
// ./mqttpub -c <client name> -i <ip address> -p <port> -t <topic> -n <loop count>
//
// e.g.
// ./mqttpub -i 192.168.1.38 -t mbed/fishtank -c MacBook -n 5
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mqtt.h"

#include <unistd.h> // for sleep function

const char *client_name = "default_pub"; 	// -c
const char *ip_addr     = "127.0.0.1";		// -i
uint32_t    port        = 1883;			// -p
const char *topic       = "hello/world";	// -t
uint32_t    count       = 10;			// -n

void parse_options(int argc, char** argv);

int main (int argc, char** argv)
{   
    puts("MQTT PUB Test Code");
    if(argc > 1) {
	parse_options(argc, argv);
    }

//  mqtt_broker_handle_t *broker = mqtt_connect("default_pub","127.0.0.1", 1883);
    mqtt_broker_handle_t *broker = mqtt_connect(client_name, ip_addr, port);

    
    if(broker == 0) {
        puts("Failed to connect");
        exit(1);
    }
 
    char msg[128];
 
    for(int i = 1; i <= count; ++i) {
        sprintf(msg, "Message number %d", i);
        if(mqtt_publish(broker, topic, msg, QoS1) == -1) {
            printf("publish failed\n");
        }
        else {
            printf("Sent %d messages\n", i);
        }
        sleep(1);
    }
    mqtt_disconnect(broker);
}

void parse_options(int argc, char** argv)
{
   for(int i = 1; i < argc; ++i) {
	if(strcmp("-c",argv[i]) == 0) {
		printf("client:%s ",argv[++i]);
		client_name = argv[i];
	}	
	if(strcmp("-i",argv[i]) == 0) {
		printf("ip:%s ",argv[++i]);
		ip_addr = argv[i];
	}
	if(strcmp("-p",argv[i]) == 0) {
		printf("port:%s ", argv[++i]);
		port = atoi(argv[i]);
	}
	if(strcmp("-t",argv[i]) == 0) {
		printf("topic:%s ",argv[++i]);
		topic = argv[i];
	}
	if(strcmp("-n",argv[i]) == 0) {
		printf("count:%s ",argv[++i]);
		count = atoi(argv[i]);
	}
   }
   puts("");
}

