//
//  mqtt.h
//  mqtt_publisher
//
//  Created by Niall Cooling on 10/01/2012.
//  Copyright 2012 __MyCompanyName__. All rights reserved.
//

#ifndef mqtt_publisher_mqtt_h
#define mqtt_publisher_mqtt_h

#include <stdint.h>

typedef struct mqtt_broker_handle mqtt_broker_handle_t;

mqtt_broker_handle_t * mqtt_connect(const char* client, const char * server_ip, uint32_t port);
void mqtt_disconnect(mqtt_broker_handle_t *broker);

int mqtt_publish(mqtt_broker_handle_t *broker, const char *topic, const char *msg);

int mqtt_subscribe(mqtt_broker_handle_t *broker, const char *topic);
void mqtt_display_message(mqtt_broker_handle_t *broker, int (*print)(int));



#endif
