#ifndef SENSOR_CONSTANTS_H
#define SENSOR_CONSTANTS_H


/* Fire detector sensor constants */
#define FIRE_ALARM	                            "{\"fire_detected\":true}"
#define SECONDS_ALARM_PERIOD 	                
#define JSON_ALARM_ON	                        "{\"mode\":\"on\"}"
#define JSON_ALARM_OFF	                        "{\"mode\":\"off\"}"

/* Servers constants */
#define SERVER_EP                               "coap://[fd00::1]:5683"
#define CONNECTION_TRY_INTERVAL                 1
#define REGISTRATION_TRY_INTERVAL               1
#define SIMULATION_INTERVAL                     1
#define FIRE_DETECTOR_SENSOR                    "fire_detector"
#define CHECK_PERIOD     CLOCK_SECOND
#define JSON_FIRE_DETECT	                    "{\"deviceType\":\"fire_detector\"}"
#define JSON_OXYGEN	                            "{\"deviceType\":\"oxygen_controller\"}"

/* Oxygen sensor server constants */
#define OXYGEN_SENSOR                           "oxygen_sensor"
#define JSON_OX_ON_FAST	                        "{\"mode\":\"on\", \"type\":\"fast\"}"
#define JSON_OX_ON_SLOW	                        "{\"mode\":\"on\", \"type\":\"slow\"}"
#define JSON_OX_OFF	                            "{\"mode\":\"off\"}"



#endif /* SENSOR_CONSTANTS_H */