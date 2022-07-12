#include <stdio.h>
#include <stdlib.h>
#include "coap-engine.h"
#include <time.h>
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
         "title=\"Fire detector\", POST alarm_mode=on|off\";obs",
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
	int len;

	if(fire_detected)
	{
		sprintf(message, "%s", FIRE_ALARM);
		data =	&message[0];
		printf("message: %s, length of the message: %d\n", message, strlen(message));
		printf("data: %s, length of the data: %d\n", data, strlen(data));

		sprintf(json_response, FIRE_ALARM);
		printf(json_response);


		coap_set_header_content_format(response, APPLICATION_JSON);
		coap_set_header_etag(response, (uint8_t *)&len, 1);
		coap_set_payload(response, data, strlen(json_response));
	}
	

}

static void
res_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *alarm_mode = NULL;
  int success = 1;

  	if((len = coap_get_post_variable(request, "alarm", &alarm_mode))) {
		LOG_DBG("alarm mode %s\n", alarm_mode);
		
		//Forse non serve ma lo lascerei se uno volesse attivare l'allarme manualmente dal controller
		if(strncmp(alarm_mode, "on", len) == 0){
			printf("Switch ON fire alarm\n");
			fire_detected = true;
			
		}else if (strncmp(alarm_mode, "off",len)==0){
			printf("Switch OFF fire alarm\n");
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

		if(!fire_detected)
			fire_detected = simulate_fire_detection();
		
		if(fire_detected){
			if(counter_fire == 0){
				coap_notify_observers(&fire_detector);
				counter_fire++;
			}
			else if(counter_fire == SECONDS_ALARM_PERIOD){
				counter_fire = 0;
			}	
		}
}

