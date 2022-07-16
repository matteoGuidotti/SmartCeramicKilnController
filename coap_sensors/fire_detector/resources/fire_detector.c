#include <stdio.h>
#include <stdlib.h>
#include "coap-engine.h"
#include <time.h>
#include "dev/leds.h"
#include <../utils/constants.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

static void get_fire_detection_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void fire_detector_event_handler(void);

//Counter for fire's alarm timing
static int counter_fire = 0;


EVENT_RESOURCE(fire_detector,
         "title=\"Fire detector\";obs",
         get_fire_detection_handler,
         res_post_handler,
         NULL,
         NULL,
         fire_detector_event_handler);


static bool fire_detected = false;
char json_response[512];

/*---------------------------------------------------------------------------*/
//returns true if the fire is detected, false otherwise
static bool simulate_fire_detection(){
	
	//the fire raises with a probability of 1%
	if(rand()%100 < 1){
		printf("Fire detected!\n");
		return true;
	}
	else{
		printf("No fire detected\n"); 
		return false;
	}
}


static void get_fire_detection_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	int len;

	if(fire_detected)
	{
		sprintf(json_response, FIRE_ALARM);
		coap_set_header_content_format(response, APPLICATION_JSON);
		coap_set_header_etag(response, (uint8_t *)&len, 1);
		coap_set_payload(response, json_response, strlen(json_response));
	}
	else{
	
		sprintf(json_response, "{\"fire_detected\": false}");
  
  		coap_set_header_content_format(response, APPLICATION_JSON);
        coap_set_header_etag(response, (uint8_t *)&len, 1);
 		coap_set_payload(response, json_response, strlen(json_response));
	}
}

//coap-client -m POST|PUT coap://[fd00::20x:x:x:x]/fire_detector -e {"alarm":"start"}|{"alarm":"stop"}

static void res_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const uint8_t* payload = NULL;
  int success = 1;

	if((len = coap_get_payload(request, &payload))) 
	{
		if(strcmp((char*)payload, JSON_START_ALARM) == 0){
			printf("Switch ON fire alarm\n");
			leds_single_off(LEDS_RED);
			leds_single_off(LEDS_GREEN);
			leds_on(LEDS_RED);
			fire_detected = true;
			counter_fire = 0;
			
		}else if(strcmp((char*)payload, JSON_STOP_ALARM) == 0){
			printf("Switch OFF fire alarm\n");
			leds_single_off(LEDS_RED);
			leds_single_off(LEDS_GREEN);
			leds_single_off(LEDS_GREEN);
			fire_detected = false;
			counter_fire = 0;
			
		}else{
			printf("ERROR: UNKNOWN COMMAND\n");
			success = 0;
		}
  	} else { success = 0;}
	
	if(!success) {
		coap_set_status_code(response, BAD_REQUEST_4_00);
	}
}


static void fire_detector_event_handler(void)
{

		
		if(!fire_detected){
			fire_detected = simulate_fire_detection();
		}
			
		
		if(fire_detected){
			leds_single_off(LEDS_RED);
			leds_single_off(LEDS_GREEN);
			leds_on(LEDS_RED);
			if(counter_fire == 0){
				coap_notify_observers(&fire_detector);
				printf("Alarm notified!\n");
				counter_fire++;
			}
			else if(counter_fire ==  SECONDS_ALARM_PERIOD -1 ){
				counter_fire = 0;
			}	
			else 
			{
				counter_fire++;
				
			}
			
		}
}

