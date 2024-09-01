#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#include "contiki.h"
#include "coap-engine.h"
#include "os/dev/leds.h"
#include "json_util.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

static uint8_t alert_status = 0; // 0 off, 1 on
extern void start_blinking(uint8_t status);

static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, 
                            uint16_t preferred_size, int32_t *offset);



RESOURCE(
    res_alert,
    "title=\"TorrefazioneCaffe: ?actuator_alert=0..\" POST/PUTaction=<action>\";rt=\"Control\";if=\"actuator\"",
    NULL,
    NULL,
    res_put_handler,
    NULL
);



static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    char* action = NULL; //per azione richiesta
    int len = 0; //per lunghezza payload richiesta
    const uint8_t* chunk; //puntatore ai dati 

    len = coap_get_payload(request, &chunk); //estraggo payload richiesta

    if(len > 0)
    {
        action = findJsonField_String((char*)chunk, "action"); //mi salvo in action l'azione scritta nel payload
        LOG_INFO("Alert received command: action=%s\n", action); //per debug
    }

    //gestione dell'azione
    if (action != NULL && strlen(action) != 0) {
        if ((strncmp(action, "on", len) == 0) && alert_status == 0) {
            leds_on(LEDS_RED); // alert acceso = luce rossa
            alert_status = 1;
            coap_set_status_code(response, CHANGED_2_04);
        } else if ((strncmp(action, "off", len) == 0) && alert_status == 1) {
            leds_off(LEDS_RED); // alert spento = luce rossa spenta
            alert_status = 0;
            coap_set_status_code(response, CHANGED_2_04);
        } else {
            coap_set_status_code(response, BAD_OPTION_4_02);
        }
    } else {
        coap_set_status_code(response, BAD_REQUEST_4_00);
    }

    free(action); //pulisco la memoria
}

