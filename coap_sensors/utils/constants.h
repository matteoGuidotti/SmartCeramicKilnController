#ifndef SENSOR_CONSTANTS_H
#define SENSOR_CONSTANTS_H

/* Common constants */
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

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

/* Oxygen sensor server constants */
#define OXYGEN_SENSOR                           "oxygen_sensor"



#endif /* SENSOR_CONSTANTS_H */