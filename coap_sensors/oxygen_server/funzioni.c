//Oxygen emitter
static void
res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  //const char *type = NULL;
  //const char *mode = NULL;
  const uint8_t* payload = NULL;
  int success = 1;

  //if((len = coap_get_query_variable(request, "type", &type))) {
	if((len = coap_get_payload(request, &payload))) {

		char data[20];
		strncpy(data, (char*)payload, len);	
		data[len] = '\0';	
		LOG_INFO("Received the message: %s", data);
    //LOG_DBG("type %.*s\n", (int)len, type);

	} else 
			success = 0;
   if(success && strcmp((char*)payload, JSON_OX_EMITTER_SLOW) == 0) {
		//LOG_DBG("mode %s\n", mode);
		/*if(strncmp((char*)type, "CTRL", len) == 0){
			emission_cause = CTRL;
			printf("Sono nel caso CTRL");*/
			oxygen_emitter = true;
			oxygen_filter = false;
			emission_cause = CTRL;
	}
	else if(success && strcmp((char*)payload, JSON_OX_EMITTER_FAST) == 0)
	{	
			oxygen_emitter = true;
			oxygen_filter = false;
			emission_cause = ADMIN;

	}
	else if(success && strcmp((char*)payload, JSON_OX_OFF) == 0)
	{	
			oxygen_emitter = false;

	}
	else
		success = 0;
		
	/*else if (strncmp((char*)type, "ADMIN", len) == 0){
		emission_cause = ADMIN;
		printf("Sono nel caso ADMIN");
	}
		

	if(strncmp(mode, "on", len) == 0){
		printf("Switch ON oxygen emitter with type: %s\n", (char*)type);
		oxygen_emitter = true;
		oxygen_filter = false;
		
	    }else if (strncmp(mode, "off",len)==0){
		printf("Switch OFF oxygen emitter with type: %s\n", (char*)type);
		oxygen_emitter = false;
	       
	    }else{
		printf("ERROR: UNKNOWN COMMAND\n");
		success = 0;
	    }*/
  //} else { success = 0;}
	
 if(!success) {
    coap_set_status_code(response, BAD_REQUEST_4_00);
  }
}


//Oxygen filter
static void
res_post_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
  size_t len = 0;
  const char *type = NULL;
  const char *mode = NULL;
  const uint8_t* payload = NULL;
  int success = 1;
  //char data[20];

  if((len = coap_get_query_variable(request, "type", &type))) {
        printf("Type: %s", type);
  
        if(strncmp((char*)type, "CTRL", len) == 0){
            emission_cause = CTRL;
            printf("Sono nel caso CTRL");
        }
        else if (strncmp((char*)type, "ADMIN", len) == 0){
            emission_cause = ADMIN;
            printf("Sono nel caso ADMIN");
        }
		

        if(strncmp(mode, "on", len) == 0){
            printf("Switch ON oxygen emitter with type: %s\n", (char*)type);
                oxygen_emitter = true;
                oxygen_filter = false;
            
            }else if (strncmp(mode, "off",len)==0){
                printf("Switch OFF oxygen emitter with type: %s\n", (char*)type);
                oxygen_emitter = false;
            
            }else{
                printf("ERROR: UNKNOWN COMMAND\n");
                success = 0;
            }
  } else { 
         printf("ERROR: UNKNOWN COMMAND\n");
        success = 0;
        }
	
 if(!success) {
    coap_set_status_code(response, BAD_REQUEST_4_00);
  }
}


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
  } */if((len_mode = coap_get_query_variable(request, "mode", &mode)) ) {
	printf("Mode : %s\n", (char*)mode);

		if(strncmp((char*)mode, "on", len_mode) == 0){

			if((len_type = coap_get_query_variable(request, "type", &type)) && (len_cause = coap_get_query_variable(request, "cause", &cause))) {

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

				printf("Mode : %s\n", (char*)mode);
				if((len_type = coap_get_query_variable(request, "type", &type))) {

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