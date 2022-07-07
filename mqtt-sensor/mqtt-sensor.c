// MQTT sensor that publishes the current temperature value and receives the desired temperature

/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "net/routing/routing.h"
#include "mqtt.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "dev/button-hal.h"
#include "dev/leds.h"
#include "os/sys/log.h"

#include <string.h>
#include <strings.h>
/*---------------------------------------------------------------------------*/
#define LOG_MODULE "mqtt-sensor"
#ifdef MQTT_CLIENT_CONF_LOG_LEVEL
#define LOG_LEVEL MQTT_CLIENT_CONF_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_DBG
#endif
/*---------------------------------------------------------------------------*/
/* MQTT broker address. */
#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1"

static char *broker_ip = MQTT_CLIENT_BROKER_IP_ADDR;

// Defaukt config values
#define DEFAULT_BROKER_PORT         1883
#define DEFAULT_PUBLISH_INTERVAL    (30 * CLOCK_SECOND)
// We assume that the broker does not require authentication

/*---------------------------------------------------------------------------*/
/* Definition of the various states */
static uint8_t state;

#define STATE_INIT    		  0
#define STATE_NET_OK    	  1
#define STATE_CONNECTING      2
#define STATE_CONNECTED       3
#define STATE_SUBSCRIBED      4
#define STATE_DISCONNECTED    5

/*---------------------------------------------------------------------------*/
PROCESS_NAME(mqtt_sensor_process);
AUTOSTART_PROCESSES(&mqtt_sensor_process);

/*---------------------------------------------------------------------------*/
/* Maximum TCP segment size for outgoing segments of our socket */
#define MAX_TCP_SEGMENT_SIZE    32
#define CONFIG_IP_ADDR_STR_LEN   64
/*---------------------------------------------------------------------------*/

// Periodic timer to check the temperature (30 seconds)
#define MEASUREMENT_PERIOD     CLOCK_SECOND * 30
static struct etimer periodic_timer;

/*---------------------------------------------------------------------------*/
/*
 * The main MQTT buffers.
 */
#define APP_BUFFER_SIZE 512
static char app_buffer[APP_BUFFER_SIZE];
/*---------------------------------------------------------------------------*/
static struct mqtt_message *msg_ptr = 0;

static struct mqtt_connection conn;

/*---------------------------------------------------------------------------*/
/*
 * Buffers for Client ID and Topics.
 * Make sure they are large enough to hold the entire respective string
 */
#define BUFFER_SIZE 64

static char client_id[BUFFER_SIZE];
//topic in which the sensor will publish
static char pub_topic[BUFFER_SIZE];
//topic to which the sensor has to be subscribed
static char sub_topic[BUFFER_SIZE];

//The maximum reachable temperature is MAX_TEMP and the mininmum one is MIN_TEMP
#define MAX_TEMP 2000
//MIN_TEMP is an approximation of the ambient possible temperature
#define MIN_TEMP 10
static float current_temperature = MIN_TEMP;
//when the heater is on, the temperature tends to encrease, decrease otherwise
static bool heater_on = false;
/*---------------------------------------------------------------------------*/

static void simulate_temperature_change(){
	//the temperature raises with a probability of 1/2 when the heater is ON
	if(heater_on){
		if(current_temperature >= MAX_TEMP)
			return;
		else{
			if(rand()%10 < 5)
				current_temperature++;
		}
	}
	//the temperature decreases with a probability of 1/5 when the heater is OFF
	else {
		if(current_temperature <= MIN_TEMP)
			return;
		else {
			if(rand()%10 < 2)
				current_temperature--;
		}
	}
}

/*---------------------------------------------------------------------------*/
PROCESS(mqtt_sensor_process, "MQTT Client");

/*---------------------------------------------------------------------------*/

static void received_chunk_handler(const char* topic, const uint8_t* chunk){
	printf("New value at topic %s has been received: %s\n", topic, chunk);

	if(strcmp(chunk, "ON") == 0)
		heater_on = true;
	else if (strcmp(chunk, "OFF") == 0)
		heater_on = false;
	else
		printf("Received unrecognised command, the heater state remains the same\n");
	return;
}

static void mqtt_event(struct mqtt_connection* m, mqtt_event_t event, void* data){
	switch(event){
		case MQTT_EVENT_CONNECTED:
			printf("The application has a MQTT connection\n");
			state = STATE_CONNECTED;
			break;
		case MQTT_EVENT_DISCONNECTED:
			printf("MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));
			state = STATE_DISCONNECTED;
			process_poll(&mqtt_sensor_process);
			break;
		case MQTT_EVENT_PUBLISH:
			msg_ptr = data;
			received_chunk_handler(msg_ptr -> topic, msg_ptr -> payload_chunk);
			break;
		case MQTT_EVENT_SUBACK:
		#if MQTT_311
			mqtt_suback_event_t *suback_event = (mqtt_suback_event_t *)data;
			if(suback_event->success) {
			printf("Application is subscribed to topic successfully\n");
			} else {
			printf("Application failed to subscribe to topic (ret code %x)\n", suback_event->return_code);
			}
		#else
			printf("Application is subscribed to topic successfully\n");
		#endif
			break;
		case MQTT_EVENT_UNSUBACK:
			printf("Application is unsubscribed to topic successfully\n");
			break;
		case MQTT_EVENT_PUBACK:
			printf("Publishing complete.\n");
			break;
		default:
			printf("Application got a unhandled MQTT event: %i\n", event);
			break;
	}
}

static bool have_connectivity(){
	if(uip_ds6_get_global(ADDR_PREFERRED) == NULL || uip_ds6_defrt_choose() == NULL)
    	return false;
  	else
		return true;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(mqtt_sensor_process, ev, data){
	PROCESS_BEGIN();

	mqtt_status_t status;
	//Initializing the topics
	strcpy(pub_topic, "current_air_temperature");
	strcpy(sub_topic, "target_air_temperature");

	printf("MQTT client sensor process\n");

	// Initialize the ClientID as MAC address
  	snprintf(client_id, BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
					linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
					linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
					linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);
	
	mqtt_register(&conn, &mqtt_client_process, client_id, mqtt_event,
                  MAX_TCP_SEGMENT_SIZE);

	//Setting the initial state 
  	state=STATE_INIT;
				    
	// Initialize periodic timer to check the status 
	etimer_set(&periodic_timer, MEASUREMENT_PERIOD);

	while(1){
		PROCESS_YIELD();

		if((ev == PROCESS_EVENT_TIMER && data == &periodic_timer) || ev == PROCESS_EVENT_POLL){
			if(state == STATE_INIT){
				if(have_connectivity() == true)
					state = STATE_NET_OK;
			}

			if(state == STATE_NET_OK){
				//Connect to MQTT server
				printf("Connecting to the server\n");
				mqtt_connect(&conn, broker_ip, DEFAULT_BROKER_PORT, (DEFAULT_PUBLISH_INTERVAL*3)/CLOCK_SECOND, MQTT_CLEAN_SESSION_ON);
				state = STATE_CONNECTING;
			}

			if(state == STATE_CONNECTED){
				//Subscribe to a topic
				status = mqtt_subscribe(&conn, NULL, sub_topic, MQTT_QOS_LEVEL_0);

				if(status == MQTT_STATUS_OUT_QUEUE_FULL){
					LOG_ERR("Tried to subscribe but command queue was full!\n");
					PROCESS_EXIT();
				}
				state = STATE_SUBSCRIBED;
			}

			if(state == STATE_SUBSCRIBED){
				char heater_state[4];
				if(heater_on)
					strcpy(heater_state, "ON");
				else
					strcpy(heater_state, "OFF");
				sprintf(app_buffer, "Current temperature is %d °C, heater is %s", current_temperature, heater_state);
				simulate_temperature_change();
				status = mqtt_publish(&conn, NULL, pub_topic, (uint8_t*) app_buffer, strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
				if(status != MQTT_STATUS_OK){
					LOG_ERR("Error during publishing registration message\n");
					switch (status){
						case  MQTT_STATUS_OUT_QUEUE_FULL:
						LOG_ERR("Error: MQTT_STATUS_OUT_QUEUE_FULL\n");
						break;
						case  MQTT_STATUS_NOT_CONNECTED_ERROR:
						LOG_ERR("Error: MQTT_STATUS_NOT_CONNECTED_ERROR\n");
						break;
						case   MQTT_STATUS_INVALID_ARGS_ERROR:
						LOG_ERR("Error:  MQTT_STATUS_INVALID_ARGS_ERROR\n");
						break;
						case   MQTT_STATUS_DNS_ERROR:
						LOG_ERR("Error:  MQTT_STATUS_DNS_ERROR\n");
						break;
						default:
						// It should never enter default case. 
						LOG_ERR("Error:  Unknown\n"); 
						break;
					}
				}
			}

			if(state == STATE_DISCONNECTED){
				LOG_ERR("Disconnected form MQTT broker\n");	
				//when the application is disconnected, it tries to reconnect to the broker
		   		state = STATE_INIT;
			}

			etimer_set(&periodic_timer, MEASUREMENT_PERIOD);
		}
	}

	PROCESS_END();
}