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

#define BLINK_INTERVAL 1000000 // Intervallo di blinking in microsecondi (1 secondo)

/*
static void blink_led(uint8_t times) {
    for (uint8_t i = 0; i < times; ++i) {
        leds_on(LEDS_RED); // Accende il LED
        clock_delay_usec(BLINK_INTERVAL / 2); // Attende metà intervallo
        leds_off(LEDS_RED); // Spegne il LED
        clock_delay_usec(BLINK_INTERVAL / 2); // Attende l'altra metà intervallo
    }
}
*/

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
        LOG_INFO("received command: action=%s\n", action); //per debug
    }

    //gestione dell'azione
    if (action != NULL && strlen(action) != 0) {
        if ((strncmp(action, "on", len) == 0) && alert_status == 0) {
            //blink_led(2); // Chiama la funzione di blinking con 5 cicli di accensione e spegnimento
            leds_set(LEDS_RED);
            alert_status = 1; //luce accesa
            coap_set_status_code(response, CHANGED_2_04);
        } else if ((strncmp(action, "off", len) == 0) && alert_status == 1) {
            leds_off(LEDS_RED); //luce spenta 
            alert_status = 0; //spengo
            coap_set_status_code(response, CHANGED_2_04);
        } else {
            coap_set_status_code(response, BAD_OPTION_4_02);
        }
    } else {
        coap_set_status_code(response, BAD_REQUEST_4_00);
    }

    free(action); //pulisco la memoria
}

