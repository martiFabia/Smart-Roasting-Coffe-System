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
#define LOG_LEVEL LOG_LEVEL_INFO

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

static struct etimer blink_timer;
static uint8_t blinking_leds = 0; // Bits per indicare quali LED stanno lampeggiando

PROCESS(blink_process, "Blink process");
AUTOSTART_PROCESSES(&blink_process);

void start_blinking(uint8_t status)
{
    uint8_t led_bit = (status == 1) ? LEDS_RED :
                      (status == 2) ? LEDS_GREEN :
                      (status == 3) ? LEDS_YELLOW : 0;

    // Aggiorna la maschera dei LED che devono lampeggiare
    if (status == 0) {
        blinking_leds = 0; // Spegni tutto se lo stato è off
    } else {
        blinking_leds |= led_bit; // Aggiungi il LED alla maschera
    }

    // Avvia il timer per il lampeggiamento se necessario
    if (blinking_leds != 0) {
        etimer_set(&blink_timer, CLOCK_SECOND / 2); // Imposta il timer per il lampeggiamento
    } else {
        etimer_stop(&blink_timer); // Ferma il timer se non ci sono più LED da lampeggiare
    }
}

PROCESS_THREAD(blink_process, ev, data)
{
    PROCESS_BEGIN();
    
    while (1) {
        PROCESS_WAIT_EVENT_UNTIL(ev == PROCESS_EVENT_TIMER && data == &blink_timer);

        // Lampeggiamento alternato dei LED
        if (blinking_leds & LEDS_RED) {
            leds_toggle(LEDS_RED);
        }
        if (blinking_leds & LEDS_GREEN) {
            leds_toggle(LEDS_GREEN);
        }
        if (blinking_leds & LEDS_YELLOW) {
            leds_toggle(LEDS_YELLOW);
        }

        etimer_reset(&blink_timer); // Reset del timer per continuare il lampeggiamento
    }

    PROCESS_END();
}

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
