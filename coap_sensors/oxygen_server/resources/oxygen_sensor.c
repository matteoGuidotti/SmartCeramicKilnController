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
#define JSON_OX_EMITTER_SLOW "{\"type\":\"emitter\", \"cause\": \"ADMIN\", \"mode\":\"on\"}"
#define JSON_OX_EMITTER_FAST "{\"type\":\"emitter\", \"cause\": \"CTRL\", \"mode\":\"on\"}"
#define JSON_OX_FILTER_FAST "{\"type\":\"filter\", \"cause\": \"ADMIN\", \"mode\":\"on\"}"
#define JSON_OX_FILTER_SLOW "{\"type\":\"filter\", \"cause\": \"CTRL\", \"mode\":\"on\"}"
#define JSON_OX_EMITTER_OFF "{\"type\":\"emitter\", \"cause\": \"ADMIN\", \"mode\":\"off\"}"
#define JSON_OX_FILTER_OFF "{\"type\":\"filter\", \"cause\": \"ADMIN\", \"mode\":\"off\"}"


/* A simple actuator example, depending on the type query parameter and post variable mode, the actuator is turn on or off */

static void res_put_post_handler2(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void get_oxygen_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
//static void res_put_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void oxygen_event_handler(void);

EVENT_RESOURCE(oxygen_sensor,
         "title=\"Oxygen sensor;obs",
         get_oxygen_handler,
         res_put_post_handler2,
         res_put_post_handler2,
         NULL,
         oxygen_event_handler);


static double oxygen_level = 21.0;
static double old_oxygen_level = 21.0;
static bool oxygen_emitter = false;
static bool oxygen_filter = false;
static bool oxygen_fast = false;

enum Risk{LOW, MEDIUM_LOW, MEDIUM, HIGH};
static enum Risk current_risk = LOW;

enum Cause{CTRL, FIRE, ADMIN};
static enum Cause filtration_cause  = CTRL;
static enum Cause emission_cause  = CTRL;

char json_response[512];

//coap-client -m POST|PUT coap://[fd00::202:2:2:2]/oxygen_sensor?type=emitter&cause=ADMIN|CTRL&mode=on|off
//coap-client -m POST|PUT coap://[fd00::202:2:2:2]/oxygen_sensor?type=filter&cause=ADMIN|CTRL&mode=on|off
static char cause[10];

static void res_put_post_handler2(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const uint8_t* payload = NULL;
  int success = 1;
  strcpy(cause, "");
  printf("POST arrivata\n");

	if((len = coap_get_payload(request, &payload))) 
	{
		printf("il payload: %s\n", payload);
		if(success && strcmp((char*)payload, JSON_OX_EMITTER_SLOW) == 0) {
		
			oxygen_emitter = true;
			oxygen_filter = false;
			emission_cause = CTRL;
			LOG_INFO("Oxygen emitter turn on in CTRL (slow) mode! %s \n", cause);
		}
		else if(success && strcmp((char*)payload, JSON_OX_EMITTER_FAST) == 0)
		{	
				oxygen_emitter = true;
				oxygen_filter = false;
				emission_cause = ADMIN;
				LOG_INFO("Oxygen emitter turn on in ADMIN (fast) mode! %s \n", cause);

		}
		else if(success && strcmp((char*)payload, JSON_OX_FILTER_FAST) == 0)
		{	
				oxygen_filter = true;
				oxygen_fast = false;
				filtration_cause = FIRE;
				LOG_INFO("Oxygen emitter turn on in FIRE (fast) mode! %s \n", cause);

		}
		else if(success && strcmp((char*)payload, JSON_OX_FILTER_SLOW) == 0)
		{	
				oxygen_filter = true;
				oxygen_fast = false;
				filtration_cause = CTRL;
				LOG_INFO("Oxygen emitter turn on in CTRL (slow) mode! %s \n", cause);

		}
		else if(success && strcmp((char*)payload, JSON_OX_EMITTER_OFF) == 0)
		{	
				oxygen_emitter = false;
				LOG_INFO("Oxygen emitter turn off! %s \n", cause);

		}
		else if(success && strcmp((char*)payload, JSON_OX_FILTER_OFF) == 0)
		{	
				oxygen_filter = false;
				LOG_INFO("Oxygen filter turn off! %s \n", cause);

		}
		
		else{
			printf("ERROR: UNKNOWN COMMAND\n");
			success = 0;
		}

	}
	else{
		printf("ERROR: UNKNOWN COMMAND\n");
		success = 0;
	}
		
	if(!success) {
		coap_set_status_code(response, BAD_REQUEST_4_00);
	}
}

/*
//Oxygen filter and emitter
static void
res_put_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len_type = 0;
  size_t len_mode = 0;
  size_t len_cause = 0;
  const char *type = NULL;
  const char *mode = NULL;
  const char *cause = NULL;
  int success = 1;

	printf("arrivata POST\n");
   if((len_mode = coap_get_query_variable(request, "mode", &mode))) {

		if(strncmp(mode, "on", len_mode) == 0){

            if((len_type = coap_get_query_variable(request, "type", &type))) {
					
					if((len_cause = coap_get_query_variable(request, "cause", &cause))) {
						
						//Filtering case
						if(strncmp(type, "filter", len_type) == 0){
							oxygen_filter = true;
							oxygen_emitter = false;

							if(strncmp((char*)cause, "CTRL", len_cause) == 0){
								filtration_cause = CTRL;
								LOG_INFO("Oxygen filter turn on in CTRL (slow) mode! %s \n", cause);
								printf("Sono nel caso filter CTRL\n");
							}
							else if(strncmp((char*)cause, "FIRE", len_cause) == 0){
								filtration_cause = FIRE;
								LOG_INFO("Oxygen emitter turn on in FIRE (fast) mode! %s \n", cause);
								printf("Sono nel caso filter FIRE\n");
							}
							else{
								success = 0;
								printf("Wrong param \"cause\"!\n");

							}
						}
						//Emission case
						else if(strncmp(type, "emitter", len_type) == 0){
							oxygen_emitter = true;
							oxygen_filter = false;

							if(strncmp((char*)cause, "CTRL", len_cause) == 0){
								emission_cause = CTRL;
								LOG_INFO("Oxygen emitter turn on in CTRL (slow) mode! %s \n", cause);
								
							}
							else if(strncmp((char*)cause, "ADMIN", len_cause) == 0){
								emission_cause = ADMIN;
								LOG_INFO("Oxygen emitter turn on in ADMIN (fast) mode! %s \n", cause);
								
							}
							else{
								success = 0;
								printf("Wrong param \"cause\"!\n");

							}
						}
						else{
							success = 0;
							printf("Wrong param \"type\"!\n");
						}
					}
					else{
						success = 0;
						printf("Missing param \"cause\"!\n");
					}
			}
			else{
				success = 0;
				printf("Wrong param \"type\"!\n");
			}
		}

		else if(strncmp(mode, "off", len_mode) == 0){
			if((len_type = coap_get_query_variable(request, "type", &type))) {

				if(strncmp(type, "filter", len_type) == 0){
						oxygen_filter = false;
					}
					else if(strncmp(type, "emitter", len_type) == 0)
					{
						oxygen_emitter = false;
					}
					else{
						success = 0;
						printf("Wrong param \"type\"!\n");
						}
			}
			else{
				success = 0;
				printf("Missing param \"type\"!\n");
			}
		}
		else{
			success = 0;
			printf("Wrong param \"mode\"!\n");
		}
   	}
	else{
		success = 0;
		printf("Missing param \"mode\"!\n");
	}
			
 
 
 if(!success) {
    coap_set_status_code(response, BAD_REQUEST_4_00);
  }
}
*/



/*---------------------------------------------------------------------------*/

static enum Risk simulate_oxygen_change(){

	int type = 0;
	double variation = (double)(rand() % 5) / 10;
	double variationFast = 0.5;
	double variationCTRL = 0.1;
	
	old_oxygen_level = oxygen_level;
	

	if(oxygen_filter && filtration_cause == FIRE){
		printf("Decreasing fast\n");
		oxygen_level = oxygen_level - variationFast;
		
	} else if(oxygen_filter && filtration_cause == CTRL){
		printf("Decreasing slow\n");
		oxygen_level = oxygen_level - variationCTRL;
	}
	else if(oxygen_emitter && emission_cause == CTRL) {
		printf("Increasing slow");
		oxygen_level = oxygen_level + variationCTRL;
	}
	else if(oxygen_emitter && emission_cause == ADMIN){
		printf("Increasing fast");
		oxygen_level = oxygen_level + variationFast;
	}
	else{
			type = rand()%100;
			//We emulate the increase or decrease in oxygen assuming that there is a change in 10% of the cases
			if((rand()%100) < 10)
			{
				//With an high probability, oxygen is decreasing due to internal combustion in the furnace
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
	//char *data = NULL;
	int len;

	
	sprintf(message, "%g", oxygen_level);
	//data =	&message[0];

	sprintf(json_response, "{\"oxygen_value\": %f}", oxygen_level);
	printf("sto mandando %s", json_response);


	coap_set_header_content_format(response, APPLICATION_JSON);
	coap_set_header_etag(response, (uint8_t *)&len, 1);
	coap_set_payload(response, json_response, strlen(json_response));

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
			leds_set(LEDS_NUM_TO_MASK(LEDS_RED));
			leds_set(LEDS_NUM_TO_MASK(LEDS_GREEN));
			break;
		case MEDIUM:
			printf("Oxygen level: %f, medium risk\n", oxygen_level);
			leds_set(LEDS_NUM_TO_MASK(LEDS_RED));
			leds_set(LEDS_NUM_TO_MASK(LEDS_GREEN));
			break;
		case HIGH:
			printf("Oxygen level: %f, high risk\n", oxygen_level);
			leds_set(LEDS_NUM_TO_MASK(LEDS_RED));
			break;
	}
  }
  
   /*printf("Old Oxygen level: %f \n", old_oxygen_level);
	printf("Oxygen level: %f \n", oxygen_level);*/
 	
	if(old_oxygen_level != oxygen_level)
  		coap_notify_observers(&oxygen_sensor);
  
}


