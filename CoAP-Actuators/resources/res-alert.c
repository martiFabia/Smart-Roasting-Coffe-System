#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "contiki.h"
#include "coap-engine.h"
#include "os/dev/leds.h"
#include "json_util/json.util.h"

/* Log configuration */
#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_APP

static uint8_t alert_status = 0; // 0 off, 1 t, 2 u, 3 g
extern void start_blinking(uint8_t status);

static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, 
                            uint16_t preferred_size, int32_t *offset);

RESOURCE(
    res_alert,
    "title=\"TorrefazioneCaffe: ?acutaor_alert=0..\" POST/PUTaction=<action>\";rt=\"Control\";if=\"actuator\"",
    NULL,
    NULL,
    res_put_handler,
    NULL
);

static void res_put_handler(coap_message_t *request, coap_message_t *response, uint8_t *buffer, uint16_t preferred_size, int32_t *offset)
{
    char* action = NULL; // per azione richiesta
    int len = 0; // per lunghezza payload richiesta
    const uint8_t* chunk; // puntatore ai dati 

    len = coap_get_payload(request, &chunk); // estraggo payload richiesta

    if(len > 0)
    {
        action = findJsonField_String((char*)chunk, "action"); // mi salvo in action l'azione scritta nel payload
        LOG_INFO("received command: action=%s\n", action); // per debug
    }

    // Gestione dell'azione
    if (action != NULL && strlen(action) != 0) {
        uint8_t new_status = 0;
        
        if (strncmp(action, "t", len) == 0) {
            new_status = 1;
        } else if (strncmp(action, "u", len) == 0) {
            new_status = 2;
        } else if (strncmp(action, "g", len) == 0) {
            new_status = 3;
        } else if (strncmp(action, "off", len) == 0) {
            new_status = 0;
        } else {
            coap_set_status_code(response, BAD_OPTION_4_02);
            free(action);
            return;
        }

        // Aggiorna lo stato dell'alert e avvia il lampeggiamento se necessario
        if (new_status != alert_status) {
            alert_status = new_status;
            start_blinking(alert_status);
            coap_set_status_code(response, CHANGED_2_04);
        } else {
            coap_set_status_code(response, CHANGED_2_04);
        }
    } 
    else {
        coap_set_status_code(response, BAD_REQUEST_4_00);
    }

    free(action); // pulisco la memoria
}
