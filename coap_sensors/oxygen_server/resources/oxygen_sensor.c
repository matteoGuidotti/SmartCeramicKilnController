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
static double old_oxygen_level = 21.0;
static bool oxygen_emitter = false;

enum Risk{LOW, MEDIUM_LOW, MEDIUM, HIGH};
static enum Risk current_risk = LOW;

enum Emitter_mode{INC, DEC};
static enum Emitter_mode e_mode  = DEC;

char json_response[512];


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
	if(strncmp((char*)type, "INC", len) == 0){
		e_mode = INC;
		printf("Sono nel caso INC");
	}
		
	else if (strncmp((char*)type, "DEC", len) == 0){
		e_mode = DEC;
		printf("Sono nel caso DEC");
	}
		

	if(strncmp(mode, "on", len) == 0){
		printf("Switch ON oxygen emitter with type: %s\n", (char*)type);
		oxygen_emitter = true;
		
	    }else if (strncmp(mode, "off",len)==0){
		printf("Switch OFF oxygen emitter with type: %s\n", (char*)type);
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

	int type = 0;
	double variation = (double)(rand() % 10) / 10;
	
	old_oxygen_level = oxygen_level;
	//printf("%d\n", oxygen_emitter);
	
	
	// oxygen emitter ON -> oxygen rises
	// oxygen emitter OFF -> 10/100 oxygen change, 2% inc, 98% dec
	if(oxygen_emitter && e_mode == INC){
		printf("Sto incrementando");
		oxygen_level = oxygen_level + 0.3;
	} else if(oxygen_emitter && e_mode == DEC){
		printf("Sto decrementando");
		oxygen_level = oxygen_level - 0.3;
	}
	else {
		printf("A caso");
		type = rand()%100;
		if( ((rand()%100) < 10) && type < 2)
			oxygen_level = oxygen_level - variation;
		else if( ((rand()%100) < 10) && type >= 2 )
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
	int len;

	
	sprintf(message, "%g", oxygen_level);
	data =	&message[0];
	printf("message: %s, length of the message: %d\n", message, strlen(message));
	printf("data: %s, length of the data: %d\n", data, strlen(data));

	sprintf(json_response, "{\"timestamp\":%llu, \"oxygen_value\": %s}", ((unsigned long long)time(NULL))*1000, data);

	len = sizeof(json_response) -1;
      
	
    memset(data, 0, len);

    memcpy(data, json_response, strlen(json_response));
  

	coap_set_header_content_format(response, APPLICATION_JSON);
	coap_set_header_etag(response, (uint8_t *)&len, 1);
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
   	printf("Old Oxygen level: %f \n", old_oxygen_level);
	printf("Oxygen level: %f \n", oxygen_level);
 	
	if(old_oxygen_level != oxygen_level)
  		coap_notify_observers(&oxygen_sensor);
  
}


