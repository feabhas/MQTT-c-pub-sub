//  Created by Niall Cooling on 10/01/2012.
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
#include <stdio.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <fcntl.h>
#include <assert.h>


#include <arpa/inet.h>
#include <netdb.h>

/* For version 3 of the MQTT protocol */
#include "mqtt.h"

#define PROTOCOL_NAME       "MQIsdp"    // 
#define PROTOCOL_VERSION    3U          // version 3.0 of MQTT
#define CLEAN_SESSION       (1U<<1)
#define KEEPALIVE           30U         // specified in seconds
#define MESSAGE_ID  1U // not used by QoS 0 - value must be > 0

/* Macros for accessing the MSB and LSB of a uint16_t */
#define MSB(A) ((uint8_t)((A & 0xFF00) >> 8))
#define LSB(A) ((uint8_t)(A & 0x00FF))


#define SET_MESSAGE(M) ((uint8_t)(M << 4))
#define GET_MESSAGE(M) ((uint8_t)(M >> 4))

typedef enum {
    CONNECT = 1,
    CONNACK,
    PUBLISH,
    PUBACK,
    PUBREC,
    PUBREL,
    PUBCOMP,
    SUBSCRIBE,
    SUBACK,
    UNSUBSCRIBE,
    UNSUBACK,
    PINGREQ,
    PINGRESP,
    DISCONNECT 
} connect_msg_t;

typedef enum {
    Connection_Accepted,
    Connection_Refused_unacceptable_protocol_version,
    Connection_Refused_identifier_rejected,
    Connection_Refused_server_unavailable,
    Connection_Refused_bad_username_or_password,
    Connection_Refused_not_authorized
} connack_msg_t;


struct mqtt_broker_handle
{
	int socket;
	struct sockaddr_in socket_address;
	uint16_t port;
	char hostname[128];
	char clientid[32];
	bool connected;
    size_t topic;
};

typedef struct fixed_header_t
{
    uint16_t     retain             : 1;
    uint16_t     Qos                : 2;
    uint16_t     DUP                : 1;
    uint16_t     connect_msg_t      : 4;
    uint16_t     remaining_length   : 8;    
}fixed_header_t;



mqtt_broker_handle_t * mqtt_connect(const char* client, const char * server_ip, uint32_t port)
{
    mqtt_broker_handle_t *broker = calloc(sizeof(mqtt_broker_handle_t), 1) ;
    if(broker != 0) {
        // check connection strings are within bounds
        if ( (strlen(client)+1 > sizeof(broker->clientid)) || (strlen(server_ip)+1 > sizeof(broker->hostname))) {
            fprintf(stderr,"failed to connect: client or server exceed limits\n");
            free(broker);
            return 0;  // strings too large
        }
        
        broker->port = port;
        strcpy(broker->hostname, server_ip);
        strcpy(broker->clientid, client);
        
        if ((broker->socket = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
            fprintf(stderr,"failed to connect: could not create socket\n");
            free(broker);            
            return 0;
        }
        
        // create the stuff we need to connect
        broker->connected = false;
        broker->socket_address.sin_family = AF_INET;
        broker->socket_address.sin_port = htons(broker->port); // converts the unsigned short from host byte order to network byte order
        broker->socket_address.sin_addr.s_addr = inet_addr(broker->hostname);
        
        // connect
        if ((connect(broker->socket, (struct sockaddr *)&broker->socket_address, sizeof(broker->socket_address))) < 0) {
            fprintf(stderr,"failed to connect: to server socket\n");
            free(broker);
            return 0;
        }
        
        // variable header
        uint8_t var_header[] = {
            0,                         // MSB(strlen(PROTOCOL_NAME)) but 0 as messages must be < 127
            strlen(PROTOCOL_NAME),     // LSB(strlen(PROTOCOL_NAME)) is redundant 
            'M','Q','I','s','d','p',
            PROTOCOL_VERSION,
            CLEAN_SESSION,
            MSB(KEEPALIVE),
            LSB(KEEPALIVE),
            0,                          // MSB(strlen(broker->clientid)) but doesn't work for > 127
            strlen(broker->clientid)    // LSB(strlen(broker->clientid))
        };
        
        // fixed header: 2 bytes, big endian
        uint8_t fixed_header[] = { SET_MESSAGE(CONNECT), sizeof(var_header)+strlen(broker->clientid) };
//      fixed_header_t  fixed_header = { .QoS = 0, .connect_msg_t = CONNECT, .remaining_length = sizeof(var_header)+strlen(broker->clientid) };
        
        uint8_t packet[sizeof(fixed_header)+sizeof(var_header)+strlen(broker->clientid)];
        
        memset(packet,0,sizeof(packet));
        memcpy(packet,fixed_header,sizeof(fixed_header));
        memcpy(packet+sizeof(fixed_header),var_header,sizeof(var_header));
        memcpy(packet+sizeof(fixed_header)+sizeof(var_header),broker->clientid,strlen(broker->clientid));
        
        // send Connect message
        if (send(broker->socket, packet, sizeof(packet), 0) < sizeof(packet)) {
            close(broker->socket);
            free(broker);
            return 0;
        }
        
        uint8_t buffer[4];
        long sz = recv(broker->socket, buffer, sizeof(buffer), 0);  // wait for CONNACK
        //        printf("buffer size is %ld\n",sz);
        //        printf("%2x:%2x:%2x:%2x\n",(uint8_t)buffer[0],(uint8_t)buffer[1],(uint8_t)buffer[2],(uint8_t)buffer[3]);
        if( (GET_MESSAGE(buffer[0]) == CONNACK) && (buffer[3] == Connection_Accepted) ) {
            printf("Connected to MQTT Server at %s:%4d\n",server_ip, port );
        }
        else
        {
            fprintf(stderr,"failed to connect with error: %d\n", buffer[3]);
            close(broker->socket);
            free(broker);
            return 0;
        }
        
        // set connected flag
        broker->connected = true;
        
    }
	
	return broker;
}




int mqtt_subscribe(mqtt_broker_handle_t *broker, const char *topic)
{
	if (!broker->connected) {
        return -1;
	}
	
	uint8_t var_header[] = { MSB(MESSAGE_ID), LSB(MESSAGE_ID) };	// appended to end of PUBLISH message
    
	// utf topic
	uint8_t utf_topic[2+strlen(topic)+1]; // 2 for message size + 1 for '\0'
    
    // set up topic payload
	utf_topic[0] = 0;                       // MSB(strlen(topic));
	utf_topic[1] = LSB(strlen(topic));
    strcpy((char *)&utf_topic[2], topic);
	utf_topic[sizeof(utf_topic)-1] = '\0';  // terminate str correctly
    
    uint8_t fixed_header[] = { SET_MESSAGE(SUBSCRIBE), sizeof(var_header)+sizeof(utf_topic)};
//    fixed_header_t  fixed_header = { .QoS = 0, .connect_msg_t = SUBSCRIBE, .remaining_length = sizeof(var_header)+strlen(utf_topic) };
	
	uint8_t packet[sizeof(fixed_header)+sizeof(var_header)+sizeof(utf_topic)];
    
	memset(packet, 0, sizeof(packet));
	memcpy(packet, &fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), var_header, sizeof(var_header));
	memcpy(packet+sizeof(fixed_header)+sizeof(var_header), utf_topic, sizeof(utf_topic));
	
	if (send(broker->socket, packet, sizeof(packet), 0) < sizeof(packet)) {
        puts("failed to send subscribe message");
		return -1;
    }
    
    uint8_t buffer[5];
    long sz = recv(broker->socket, buffer, sizeof(buffer), 0);  // wait for SUBACK
    
    //    printf("buffer size is %ld\n",sz);
    //    printf("%2x:%2x:%2x:%2x:%2x\n",(uint8_t)buffer[0],(uint8_t)buffer[1],(uint8_t)buffer[2],(uint8_t)buffer[3],(uint8_t)buffer[4]);
    
    if( (GET_MESSAGE(buffer[0]) == SUBACK) && (buffer[2] == MSB(MESSAGE_ID)) &&  (buffer[3] == LSB(MESSAGE_ID)) ) {
        printf("Subscribed to MQTT Service %s\n", topic);
    }
    else
    {
        puts("failed to subscribe");
        return -1;
    }
    broker->topic = strlen(topic);
	return 1;
}


void mqtt_display_message(mqtt_broker_handle_t *broker, int (*print)(int))
{
    uint8_t buffer[128];
    while(1) {
        // wait for next message
        long sz = recv(broker->socket, buffer, sizeof(buffer), 0);
        
        // if more than ack - i.e. data > 0
        if(sz > 0) {
            //printf("message size is %ld\n",sz);
            if( GET_MESSAGE(buffer[0]) == PUBLISH) {                
                //printf("Got PUBLISH message with payload size %d\n", (uint8_t)buffer[1]);
                for (unsigned long i = 4+(broker->topic); i < sz; ++i) { 
                    print(buffer[i]);
                }
                print('\n');
                return;
            }
        }
    }
    
}


int mqtt_publish(mqtt_broker_handle_t *broker, const char *topic, const char *msg)
{
	if (!broker->connected) {
        return -1;
	}
	
	// utf topic
	uint8_t utf_topic[2+strlen(topic)]; // 2 for message size QoS-0 does not have msg ID
    
    // set up topic payload
	utf_topic[0] = 0;                       // MSB(strlen(topic));
	utf_topic[1] = LSB(strlen(topic));
    strcpy((char *)&utf_topic[2], topic);
		
	uint8_t fixed_header[] = { SET_MESSAGE(PUBLISH), sizeof(utf_topic)+strlen(msg)};
//    fixed_header_t  fixed_header = { .QoS = 0, .connect_msg_t = PUBLISH, .remaining_length = sizeof(utf_topic)+strlen(msg) };
    	
	uint8_t packet[sizeof(fixed_header)+sizeof(utf_topic)+strlen(msg)];
    
	memset(packet, 0, sizeof(packet));
	memcpy(packet, &fixed_header, sizeof(fixed_header));
	memcpy(packet+sizeof(fixed_header), utf_topic, sizeof(utf_topic));
	memcpy(packet+sizeof(fixed_header)+sizeof(utf_topic), msg, strlen(msg));
    
	if (send(broker->socket, packet, sizeof(packet), 0) < sizeof(packet)) {
		return -1;
    }
    
	return 1;
}

void mqtt_disconnect(mqtt_broker_handle_t *broker)
{
    uint8_t fixed_header[] = { SET_MESSAGE(DISCONNECT), 0};
    if (send(broker->socket, fixed_header, sizeof(fixed_header), 0)< sizeof(fixed_header)) {
        puts("failed to disconnect");
    }
}

