//
//  mqtt.h
//
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

#ifndef mqtt_h
#define mqtt_h

#include <stdint.h>

typedef struct mqtt_broker_handle mqtt_broker_handle_t;

typedef enum {QoS0, QoS1, QoS2} QoS;

mqtt_broker_handle_t * mqtt_connect(const char* client, const char * server_ip, uint32_t port);
void mqtt_disconnect(mqtt_broker_handle_t *broker);

int mqtt_publish(mqtt_broker_handle_t *broker, const char *topic, const char *msg, QoS qos);

int mqtt_subscribe(mqtt_broker_handle_t *broker, const char *topic, QoS qos);
void mqtt_display_message(mqtt_broker_handle_t *broker, int (*print)(int));



#endif
