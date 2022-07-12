#ifndef SENSOR_CONSTANTS_H
#define SENSOR_CONSTANTS_H

/* Fire detector sensor constants */
#define FIRE_ALARM	                            "{\"fire_detected\":true}"
#define SECONDS_ALARM_PERIOD 	                60

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



#endif /* SENSOR_CONSTANTS_H */