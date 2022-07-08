#include <stdio.h>
#include <stdlib.h>
#include "coap-engine.h"
#include "dev/leds.h"
#include <time.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

/* A simple actuator example, depending on the type query parameter and post variable mode, the actuator is turn on or off */

static void res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_oxygen_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void oxygen_event_handler(void);
EVENT_RESOURCE(oxygen_sensor,
         "title=\"Oxygen sensor: ?type=INC|DEC, POST/PUT mode=on|off\";obs",
         get_oxygen_handler,
         res_post_put_handler,
         res_post_put_handler,
         NULL,
         oxygen_event_handler);


static double oxygen_level = 21.0;
static bool oxygen_emitter = false;

enum Risk{LOW, MEDIUM_LOW, MEDIUM, HIGH};
static enum Risk current_risk = LOW;

//enum Emitter_mode{INC, DEC};
char * e_mod  = "DEC";


static void
res_post_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *type = NULL;
  const char *mode = NULL;
  int success = 1;

  if((len = coap_get_query_variable(request, "type", &type))) {
    LOG_DBG("type %.*s\n", (int)len, type);

  } else {
    success = 0;
  } if(success && (len = coap_get_post_variable(request, "mode", &mode))) {
    LOG_DBG("mode %s\n", mode);
	
	if(strcmp(mode, "on") == 0){
	        printf("Switch ON oxygen emitter\n");
	        oxygen_emitter = true;
	        e_mod = (char*)type;
	    }else if (strcmp(mode, "off")==0){
	        printf("Switch OFF oxygen emitter\n");
	        oxygen_emitter = false;
	    }else{
	        printf("ERROR: UNKNOWN COMMAND\n");
	        success = 0;
	    }
  } else { success = 0;}
	
 if(!success) {
    coap_set_status_code(response, BAD_REQUEST_4_00);
  }
}



/*---------------------------------------------------------------------------*/

static enum Risk simulate_oxygen_change(){
	srand(time(NULL));
	int type = 0;
	double variation = (double)(rand() % 10) / 10;
	// oxygen emitter ON -> oxygen rises
	// oxygen emitter OFF -> 3/10 oxygen change, 50% inc, 50% dec
	if(oxygen_emitter && strcmp("INC", e_mod) ==0){
		oxygen_level = oxygen_level + 0.3;
	} else if(oxygen_emitter && strcmp("DEC", e_mod) ==0){
		oxygen_level = oxygen_level - 0.3;
	}
	else {
		type = rand()%2;
		if( ((rand()%10) < 3) && type == 0)
			oxygen_level = oxygen_level - variation;
		else if( ((rand()%10) < 3) && type == 1)
			oxygen_level = oxygen_level + variation;
	}
	
	if (oxygen_level >= 20.0)
		return LOW;
	else if (oxygen_level < 20.0 && oxygen_level >= 15.0)
		return MEDIUM_LOW;
	else if (oxygen_level < 15.0 && oxygen_level >= 13.0)
		return MEDIUM;
	else return HIGH;
	
}



static void get_oxygen_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	char message[30];
	char *data = NULL;

	//converting the level of oxygen in string
	
	sprintf(message, "%g", oxygen_level);
	data =	&message[0];
	printf("message: %s, length of the message: %d\n", message, strlen(message));
	printf("data: %s, length of the data: %d\n", data, strlen(data));
  
	coap_set_header_content_format(response, TEXT_PLAIN);
	coap_set_payload(response, data, strlen(data));
}


static void oxygen_event_handler(void)
{
  enum Risk sensed_risk = simulate_oxygen_change();
  
  if (current_risk != sensed_risk){
	current_risk = sensed_risk;
	switch (current_risk) {
		case LOW:
			LOG_INFO("Oxygen level: %f,low risk \n", oxygen_level);
			leds_set(LEDS_NUM_TO_MASK(LEDS_GREEN));
			break;
		case MEDIUM_LOW:
			printf("Oxygen level: %f, medium-low risk\n", oxygen_level);
			leds_set(LEDS_NUM_TO_MASK(LEDS_YELLOW));
			leds_set(LEDS_NUM_TO_MASK(LEDS_GREEN));
			break;
		case MEDIUM:
			printf("Oxygen level: %f, medium risk\n", oxygen_level);
			leds_set(LEDS_NUM_TO_MASK(LEDS_YELLOW));
			break;
		case HIGH:
			printf("Oxygen level: %f, high risk\n", oxygen_level);
			leds_set(LEDS_NUM_TO_MASK(LEDS_RED));
			break;
	}
  }
  // Da rivedere
  coap_notify_observers(&oxygen_sensor);
  
}


