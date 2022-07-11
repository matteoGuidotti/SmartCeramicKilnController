#include <stdlib.h>
#include <string.h>
#include "coap-engine.h"
#include "dev/leds.h"
#include "sys/log.h"


#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

static void put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset);

RESOURCE(nitrogen_emitter,
         "title=\"Nitrogen emitter, PUT mode=on|off\";rt=\"Control\"",
         NULL,
         NULL,
         put_handler,
         NULL);


bool nitrogen_emitter = false;
//float nitrogen_level = 0.0;

static void
put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
	size_t len = 0;
	const char *mode = NULL;
	int success = 1;

	if(len = coap_get_post_variable(request, "mode", &mode)){
		if(strncmp(mode, "on", len) == 0){
			printf("Switch ON nitrogen emitter\n");
			oxygen_emitter = true;
		}
		else if (strncmp(mode, "off",len)==0){
			printf("Switch OFF oxygen emitter\n");
			oxygen_emitter = false;
	       	}
	       	else{
		printf("ERROR: UNKNOWN COMMAND\n");
		success = 0;
	    }
	}
	else
		success = 0;
		
  if(!success) 
    coap_set_status_code(response, BAD_REQUEST_4_00);  
}


