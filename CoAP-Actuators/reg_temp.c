/*
In questo file viene implementato il processo dell'attuatore "regolatore di temperatura" e la registrazione al server

LAB 03 VALLATI
*/


#include "contiki.h"
#include "coap-engine.h"
#include "coap-blocking-api.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "sys/etimer.h"
#include "os/dev/leds.h"
#include <stdio.h>

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

// Server IP and resource path
#define SERVER_EP "coap://[fd00::1]:5683"
#define NODE_NAME_JSON "{\"name\":\"actuator_reg_temp\",\"status\":\"off\"}"
#define MAX_REGISTRATION_RETRY 3

static coap_endpoint_t server_ep;
static coap_message_t request[1];       //cosi possiamo trattare il pacchetto come un puntatore
static char *service_registration_url = "/registration"; //!!!!!!!!!!
static int max_registration_retry = MAX_REGISTRATION_RETRY;

//define a handler to handle the response from the server
//gestione quindi della risposta del server al tentativo di registrazione
void client_chunk_handler(coap_message_t *response){

    if(response == NULL){
        LOG_ERR("Request timed out\n");
    }else if(response->code != CREATED_2_01){
        LOG_ERR("Errore. Codice ricevuto:
         %d\n", response->code);
    }else{ //se sono qui è andato tutto bene
        LOG_INFO("Registrazione riuscita!\n");
        max_registration_retry = 0;
        return;
    }

    //se sono qui qualcosa è andato storto
    max_registration_retry--;
    if(max_registration_retry == 0){
        max_registration_retry = -1;
    }
}

extern coap_resource_t res_reg_temp;
static struct etimer sleep_timer;

PROCESS(reg_temp_thread, "regolatore di temperatura");
AUTOSTART_PROCESSES(&reg_temp_thread);

PROCESS_THREAD(reg_temp_thread, ev, data){
	
	PROCESS_BEGIN();
   // leds_on(LEDS_RED); per dongle
   leds_set(LEDS_NUM_TO_MASK(LEDS_RED));

    while(max_registration_retry != 0){

        coap_endpoint_parse(SERVER_EP, strlen(SERVER_EP), &server_ep); //populate the endpoint data structure

        coap_init_message(request, COAP_TYPE_CON, COAP_POST, 0);
        coap_set_header_uri_path(request, service_registration_url);

        coap_set_payload(request, (uint8_t *)NODE_NAME_JSON, sizeof(NODE_NAME_JSON) - 1);
	
		COAP_BLOCKING_REQUEST(&server_ep, request, client_chunk_handler);

        // se qualcosa è andato storto, sleep di 15 secondi prima di ritentare
        if(max_registration_retry == -1){
            etimer_set(&sleep_timer, 15*CLOCK_SECOND);
            PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&sleep_timer));
			max_registration_retry = MAX_REGISTRATION_RETRY;
        }

    }

    coap_activate_resource(&res_reg_temp, "actuator_reg_temp");
	
	PROCESS_YIELD();
    PROCESS_END();

}


