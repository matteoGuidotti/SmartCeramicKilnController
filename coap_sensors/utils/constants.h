#ifndef SENSOR_CONSTANTS_H
#define SENSOR_CONSTANTS_H


/* Fire detector sensor constants */
#define FIRE_ALARM	                            "{\"fire_detected\":true}"
#define SECONDS_ALARM_PERIOD 	                60
#define JSON_START_ALARM                        "{\"alarm\":\"start\"}";
#define JSON_STOP_ALARM                         "{\"alarm\":\"stop\"}";

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
#define JSON_OX_EMITTER_SLOW                    "{\"type\":\"emitter\", \"cause\": \"CTRL\", \"mode\":\"on\"}"
#define JSON_OX_EMITTER_FAST                    "{\"type\":\"emitter\", \"cause\": \"ADMIN\", \"mode\":\"on\"}"
#define JSON_OX_FILTER_FAST                     "{\"type\":\"filter\", \"cause\": \"FIRE\", \"mode\":\"on\"}"
#define JSON_OX_FILTER_SLOW                     "{\"type\":\"filter\", \"cause\": \"CTRL\", \"mode\":\"on\"}"
#define JSON_OX_EMITTER_OFF                     "{\"type\":\"emitter\", \"mode\":\"off\"}"
#define JSON_OX_FILTER_OFF                      "{\"type\":\"filter\", \"mode\":\"off\"}"



#endif /* SENSOR_CONSTANTS_H */