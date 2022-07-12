#include <stdio.h>
#include <stdlib.h>
#include "coap-engine.h"
#include <time.h>
#include <../utils/constants.h>

static void get_fire_detection_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void res_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);
static void fire_detection_event_handler(void);

//Counter for fire's alarm timing
static int counter_fire = 0;


EVENT_RESOURCE(ethylene_sensor,
         "title=\"Fire detector\", POST/PUT alarm_mode=on|off\";obs",
         get_fire_detection_handler,
         res_post_handler,
         NULL,
         NULL,
         fire_detector_event_handler);


static bool fire_detected = false;
static bool alarm_on = false;
/*---------------------------------------------------------------------------*/
//returns true if the fire is detected, false otherwise
static bool simulate_fire_detection(){
	//srand(time(NULL));
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
	char message[30];
	char *data = NULL;
	if(fire_detected)
	{
		sprintf(json_response, FIRE_ALARM);
		printf(json_response);

		coap_set_header_content_format(response, APPLICATION_JSON);
		coap_set_payload(response, data, strlen(json_response));
	}
	

}

static void
res_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *alarm_mode = NULL;
  int success = 1;

  	if(len = coap_get_post_variable(request, "alarm", &alarm_mode)) {
		LOG_DBG("alarm mode %s\n", alarm_mode);
		
		//Forse non serve ma lo lascerei se uno volesse attivare l'allarme manualmente dal controller
		if(strncmp(mode, "on", len) == 0){
			printf("Switch ON fire alarm\n");
			alarm_on = true;
			
		}else if (strncmp(mode, "off",len)==0){
			printf("Switch OFF fire alarm\n");
			alarm_on = false;
			fire_detected = false;
			
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
	
		if(fire_detected || alarm_on)
		{
			if(counter_fire == 0)
				counter_fire++;
			else if(counter_fire == ALARM_PERIOD){
				coap_notify_observers(&fire_detector);
				counter_fire = 0;
			}
			return;
		}
		
		//Da rivedere
		fire_detected = simulate_fire_detection();
		if(fire_detected)
			counter_fire = 0;
    
}

