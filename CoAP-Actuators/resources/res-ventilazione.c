/*
In questo file definisco l'attuatore "ventilazione" come risorsa e l'handler per la gestione delle richieste 
di azione.


*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "contiki.h"
#include "coap-engine.h"
#include "os/dev/leds.h"
#include "json_util.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

static uint8_t vent_status = 0; // 0 off, 1 on

static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, 
                            uint16_t preferred_size, int32_t *offset);

//uso put in quanto la ricezione del comando corrisponde ad un aggiornamento dello stato della risorsa

RESOURCE(
    res_vent,
    "title=\"TorrefazioneCaffe: ?acutaor_vent=0..\" POST/PUTaction=<action>\";rt=\"Control\";if=\"actuator\"",
    NULL,
    NULL,
    res_put_handler,
    NULL
);

static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    char* action = NULL //per azione richiesta
    int len = 0; //per lunghezza payload richiesta
    const uint8_t* chunk; //puntatore ai dati 

    len = coap_get_payload(request, &chunk); //estraggo payload richiesta

    if(len > 0)
    {
        action = findJsonField_String((char*)chunk, "action"); //mi salvo in action l'azione scritta nel payload
        LOG_INFO("received command: action=%s\n", action); //per debug
    }

    //gestione dell'azione
    if (action != NULL && strlen(action) != 0) {
        if ((strncmp(action, "on", len) == 0) && vent_status == 0) {
            leds_set(LEDS_GREEN); //luce accesa = luce verde 
            vent_status = 1; //luce accesa
            coap_set_status_code(response, CHANGED_2_04);
        } else if ((strncmp(action, "off", len) == 0) && vent_status == 1) {
            leds_set(LEDS_RED); //luce spenta = luce rossa
            vent_status = 0; //spengo
            coap_set_status_code(response, CHANGED_2_04);
        } else {
            coap_set_status_code(response, BAD_OPTION_4_02);
        }
        } else {
            coap_set_status_code(response, BAD_REQUEST_4_00);
        }

    free(action); //pulisco la memoria
}


