/*
In questo file viene implementato il processo dell'attuatore "ventilazione" e la registrazione al server
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
#define NODE_NAME_JSON "{\"name\":\"actuator_vent\",\"status\":\"off\"}"
#define MAX_REGISTRATION_RETRY 3

static coap_endpoint_t server_ep;
static coap_message_t request[1];       //cosi possiamo trattare il pacchetto come un puntatore
static char *service_registration_url = "/registration"; //!!!!!!!!!!
static int max_registration_retry = MAX_REGISTRATION_RETRY;


