#include "contiki.h"
#include "os/dev/leds.h"

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