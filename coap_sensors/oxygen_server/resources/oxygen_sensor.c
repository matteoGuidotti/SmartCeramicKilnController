#include <stdio.h>
#include <stdlib.h>
#include "coap-engine.h"
#include "dev/leds.h"
#include <time.h>
#include <../utils/constants.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP


/* A simple actuator example, depending on the type query parameter and post variable mode, the actuator is turn on or off */


static void get_oxygen_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_put_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void oxygen_event_handler(void);

EVENT_RESOURCE(oxygen_sensor,
         "title=\"Oxygen sensor;obs",
         get_oxygen_handler,
         res_put_post_handler,
         res_put_post_handler,
         NULL,
         oxygen_event_handler);


static double oxygen_level = 21.0;
static double old_oxygen_level = 21.0;
static bool oxygen_emitter = false;
static bool oxygen_filter = false;

enum Risk{LOW, MEDIUM_LOW, MEDIUM, HIGH};
static enum Risk current_risk = LOW;

enum Cause{CTRL, FIRE, ADMIN};
static enum Cause filtration_cause  = CTRL;
static enum Cause emission_cause  = CTRL;

char json_response[512];


//Oxygen filtering (post)
//"{\"mode\":\"on\", \"type\":\"CTRL\"}"
//"{\"mode\":\"on\", \"type\":\"FIRE\"}"
//"{\"mode\":\"off\"}"

//Oxygen emitter (put)
//"{\"mode\":\"off\", \"type\":\"CTRL\"}"
//"{\"mode\":\"off\", \"type\":\"ADMIN\"}"
//"{\"mode\":\"off\"}"





//Oxygen filtering and emitting
static void
res_put_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len_mode = 0;
  size_t len_type = 0;
  size_t len_cause = 0;
  const char *mode = NULL;
  const char *type = NULL;
  const char *cause = NULL;
  int success = 1;

  /*if((len = coap_get_query_variable(request, "type", &cause))) {
    LOG_DBG("type %.*s\n", (int)len, type);

  } else {
    success = 0;
  } */if((len_mode = coap_get_post_variable(request, "mode", &mode)) ) {
	sprintf("Mode : %s\n", (char*)mode);

		if(strncmp((char*)mode, "on", len_mode) == 0){

			if((len_type = coap_get_post_variable(request, "type", &type)) && (len_cause = coap_get_post_variable(request, "cause", &cause))) {

				if(strncmp((char*)type, "filter", len_type) == 0){
					if(strncmp((char*)cause, "CTRL", len_cause) == 0)
						filtration_cause = CTRL;
					else if(strncmp((char*)cause, "FIRE", len_cause) == 0)
						filtration_cause = FIRE;
					else{
						success = 0;
						printf("Param \"type\" not valid!\n");
					}
					oxygen_filter = true;
					oxygen_emitter = false;
					
				}
				else if(strncmp((char*)type, "emitter", len_type) == 0)
				{
					if(strncmp((char*)cause, "CTRL", len_cause) == 0)
						emission_cause = CTRL;
					else if(strncmp((char*)cause, "ADMIN", len_cause) == 0)
						emission_cause = ADMIN;
					else{
						success = 0;
						printf("Param \"cause\" not valid!\n");
					}
					oxygen_emitter = true;
					oxygen_filter = false;
					
				}
				else{
					success = 0;
					printf("Param \"type\" not valid!\n");
				}
			}
			else 
				success = 0;

	    }else if (strncmp(mode, "off",len_mode)==0){

				sprintf("Mode : %s\n", (char*)mode);
				if((len_type = coap_get_post_variable(request, "type", &type))) {

					if(strncmp((char*)type, "filter", len_type) == 0){
						printf("Switch OFF oxygen filtering\n");
						oxygen_filter = false;
					}
					else if(strncmp((char*)type, "emitter", len_type) == 0){
						printf("Switch OFF oxygen emitter\n");
						oxygen_emitter = false;
					}
					else{
						success = 0;
						printf("Param \"type\" not valid!\n");
					}
						
				}
				else
					success = 0;
	       
	    }
		else{
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
	double variation = (double)(rand() % 5) / 10;
	double variationFast = 0.5;
	double variationCTRL = 0.1;
	
	old_oxygen_level = oxygen_level;
	

	if(oxygen_filter && filtration_cause == FIRE){
		printf("Sto decrementando velocemente");
		oxygen_level = oxygen_level - variationFast;
		
	} else if(oxygen_filter && filtration_cause == CTRL){
		printf("Sto decrementando lentamente");
		oxygen_level = oxygen_level - variationCTRL;
	}
	else if(oxygen_emitter && emission_cause == CTRL) {
		printf("Sto incrementando lentamente");
		oxygen_level = oxygen_level + variationCTRL;
	}
	else if(oxygen_emitter && emission_cause == ADMIN){
		printf("Sto incrementando velocemente");
		oxygen_level = oxygen_level + variationFast;
	}
	else{
			type = rand()%100;
			//We emulate the increase or decrease in oxygen assuming that there is a change in 10% of the cases
			if((rand()%100) < 10)
			{
				//With a high probability, oxygen is decreasing due to internal combustion in the furnace
				if(type >= 2)
					oxygen_level = oxygen_level - variation;
				//With a low probability (2%) the oxygen level inside the furnace increases due to a failure
				else if(type < 2 )
					oxygen_level = oxygen_level + variation;
			}	
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

	
	//sprintf(message, "%g", oxygen_level);
	//data =	&message[0];
	printf("message: %s, length of the message: %d\n", message, strlen(message));
	//printf("data: %s, length of the data: %d\n", data, strlen(data));
	printf("data: %f\n", oxygen_level);

	//sprintf(json_response, "{\"timestamp\":%llu, \"oxygen_value\": %s}", ((unsigned long long)time(NULL))*1000, data);
	sprintf(json_response, "{\"oxygen_value\": %f}", oxygen_level);
	printf(json_response);


	coap_set_header_content_format(response, APPLICATION_JSON);
	coap_set_header_etag(response, (uint8_t *)&len, 1);
	coap_set_payload(response, data, strlen(json_response));

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


