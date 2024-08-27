#include "contiki.h"
#include "os/dev/button-hal.h"
#include "net/routing/routing.h"
#include "mqtt.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-icmp6.h"
#include "net/ipv6/sicslowpan.h"
#include "sys/etimer.h"
#include "sys/ctimer.h"
#include "lib/sensors.h"
#include "dev/button-hal.h"
#include "dev/leds.h"
#include "os/sys/log.h"
#include "mqtt-client.h"
#include "json_util.h"

#include <stdio.h>
#include <string.h>
#include <strings.h>

/*---------------------------------------------------------------------------*/
/* LOG settings */
#define LOG_MODULE "mqtt-client"
#ifdef MQTT_CLIENT_CONF_LOG_LEVEL
#define LOG_LEVEL MQTT_CLIENT_CONF_LOG_LEVEL
#else
#define LOG_LEVEL LOG_LEVEL_INFO
#endif

/*---------------------------------------------------------------------------*/
/* MQTT broker address */
#define MQTT_CLIENT_BROKER_IP_ADDR "fd00::1"
static const char *broker_ip = MQTT_CLIENT_BROKER_IP_ADDR;

// Default config values
#define DEFAULT_BROKER_PORT         1883
#define DEFAULT_PUBLISH_INTERVAL    (30 * CLOCK_SECOND)    

/*---------------------------------------------------------------------------*/
/* Maximum TCP segment size for outgoing segments of our socket */
#define MAX_TCP_SEGMENT_SIZE    32
#define CONFIG_IP_ADDR_STR_LEN   64

/*---------------------------------------------------------------------------*/
/* Buffers for Client ID and Topics.*/
#define BUFFER_SIZE 64

static char client_id[BUFFER_SIZE];
static char pub_topic[BUFFER_SIZE];
static char sub_topic[BUFFER_SIZE];

/*---------------------------------------------------------------------------*/
/* The main MQTT buffers.*/
#define APP_BUFFER_SIZE 512
static char app_buffer[APP_BUFFER_SIZE];

/*---------------------------------------------------------------------------*/
/* Various states */
static uint8_t state;       //tiene traccia dello stato attuale del programma 

#define STATE_INIT    		  0
#define STATE_NET_OK    	  1
#define STATE_CONNECTING      2
#define STATE_CONNECTED       3
#define STATE_SUBSCRIBED      4
#define STATE_DISCONNECTED    5

/*---------------------------------------------------------------------------*/
/* GLOBAL VARIABLES*/
static struct mqtt_message *msg_ptr = 0;    //punt. alla strutt del mess MQTT corrente 
static struct mqtt_connection conn;

// Periodic timer to check the state of the MQTT client
#define STATE_MACHINE_PERIODIC     (CLOCK_SECOND >> 1)

#define WAIT_FOR_RECONNECTION 10
static struct etimer e_timer;
static struct etimer sleep_timer;
static struct ctimer sensing_timer;

static mqtt_status_t status;
static char broker_address[CONFIG_IP_ADDR_STR_LEN];

static int value = 50;
static uint8_t min_humidity_parameter_FIRST = 40;
static uint8_t max_humidity_parameter_FIRST = 60;
static uint8_t min_humidity_parameter_SECOND = 20;
static uint8_t max_humidity_parameter_SECOND = 40;
static uint8_t min_humidity_parameter_THIRD = 10;
static uint8_t max_humidity_parameter_THIRD = 20;
static bool alarm_state = false;
static int flag_over_under = -1;     // -1 normal value, 0 over values, 1 under values
static int button_pressed = 0;

#define SENSE_PERIOD 		5		// seconds
#define SENSE_PERIOD_ON_ALERT 	3		// seconds
#define NUM_PERIOD_BEFORE_SEND  6	// every 30 seconds there's one pub

// Time tracking
static uint16_t time_elapsed = 0;
#define FIRST_INTERVAL  10          // 10 seconds
#define SECOND_INTERVAL 20          // 20 seconds
#define THIRD_INTERVAL  30          // 30 seconds

static int num_period = 0;
static int is_first_pub_flag = 1;
static int interval = 1;
button_hal_button_t *btn;
/*---------------------------------------------------------------------------*/

//funzione che calcola l'intervallo in base al tempo trascorso
static void set_interval(){

        if (time_elapsed <= FIRST_INTERVAL) {
            interval = 1; 
        
        } else if (time_elapsed > FIRST_INTERVAL && time_elapsed <= SECOND_INTERVAL) {
            interval = 2; 
        } else {
            interval = 3; 
        }

}

static int generate_random_humidity(int interval) {
    int min, max;
    if (interval == 1) {
        min = min_humidity_parameter_FIRST;
        max = max_humidity_parameter_FIRST;
    } else if (interval == 2) {
        min = min_humidity_parameter_SECOND;
        max = max_humidity_parameter_SECOND;
    } else if (interval == 3) {
        min = min_humidity_parameter_THIRD;
        max = max_humidity_parameter_THIRD;
    } else {
        return -1;  // Valore di errore
    }

    if (min >= max) {
        return -1;  // Valore di errore
    }

    return (rand() % (max - min)) + min;    //include il il min ed esclude il max
}

/* SENSING SIMULATION */
//simulazione lettura di un sensore di umidità
static int simulate_humidity_sensing(){

    set_interval();
    if(alarm_state){    //se il valore è fuori range
        if(flag_over_under == 0){   //se il valore è deve essere sopra il massimo
            if (interval == 1)
                return (rand() % 20) + max_humidity_parameter_FIRST; 
            else if (interval == 2)
                return (rand() % 20) + max_humidity_parameter_SECOND;
            else if (interval == 3) 
                return (rand() % 20) + max_humidity_parameter_THIRD;
        }else if(flag_over_under == 1){     //se il valore è deve essere sotto il minimo
            if (interval == 1)
                return rand() % min_humidity_parameter_FIRST; 
            else if (interval == 2) 
                return rand() % min_humidity_parameter_SECOND;
            else if (interval == 3)
                return rand() % min_humidity_parameter_THIRD;  
        }

    }else {                         //normal state
        return generate_random_humidity(interval);
    }
    return 0;

}

static void sense_callback(void *ptr){	
	value = simulate_humidity_sensing();
	LOG_INFO("Humidity value detected = %d %d%s", value, interval, (alarm_state)? "\t ->VALUE OUT RANGE\n":"\n");


    if(alarm_state)
        time_elapsed = (time_elapsed + SENSE_PERIOD_ON_ALERT) % 31;     //modulo 31 cosi da non superare il tempo massimo e ricominciare da 0
    else
        time_elapsed = (time_elapsed + SENSE_PERIOD) % 31;


    if(num_period >= NUM_PERIOD_BEFORE_SEND){
        sprintf(pub_topic, "%s", "sensor/humidity");	
        sprintf(app_buffer, "{ \"humidity_value\": %d, \"interval\": %d }", value, interval);
        mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
        num_period = 0;
    }
    else if(alarm_state){
        sprintf(pub_topic, "%s", "sensor/humidity");	
        sprintf(app_buffer, "{ \"humidity_value\": %d, \"interval\": %d }", value, interval);
        mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
    }
    else{
        num_period++;
    }

    
    if(alarm_state)
        ctimer_set(&sensing_timer, SENSE_PERIOD_ON_ALERT * CLOCK_SECOND, sense_callback, NULL);
    else
        ctimer_set(&sensing_timer, SENSE_PERIOD * CLOCK_SECOND, sense_callback, NULL);
}


/*---------------------------------------------------------------------------*/
PROCESS(sensor_humidity, "MQTT sensor_humidity");
AUTOSTART_PROCESSES(&sensor_humidity);

static void pub_handler_humidity(const char *topic, uint16_t topic_len, const uint8_t *chunk, uint16_t chunk_len){
    min_humidity_parameter_FIRST = (uint8_t)findJsonField_Number((char *)chunk, "min_humidity_parameter_FIRST");
    max_humidity_parameter_FIRST = (uint8_t)findJsonField_Number((char *)chunk, "max_humidity_parameter_FIRST");
    min_humidity_parameter_SECOND = (uint8_t)findJsonField_Number((char *)chunk, "min_humidity_parameter_SECOND");
    max_humidity_parameter_SECOND = (uint8_t)findJsonField_Number((char *)chunk, "max_humidity_parameter_SECOND");
    min_humidity_parameter_THIRD = (uint8_t)findJsonField_Number((char *)chunk, "min_humidity_parameter_THIRD");
    max_humidity_parameter_THIRD = (uint8_t)findJsonField_Number((char *)chunk, "max_humidity_parameter_THIRD");

    LOG_INFO("Humidity Pub Handler: First Interval min=%hhd max=%hhd\n", min_humidity_parameter_FIRST, max_humidity_parameter_FIRST);
    LOG_INFO("Humidity Pub Handler: Second Interval min=%hhd max=%hhd\n", min_humidity_parameter_SECOND, max_humidity_parameter_SECOND);
    LOG_INFO("Humidity Pub Handler: Third Interval min=%hhd max=%hhd\n", min_humidity_parameter_THIRD, max_humidity_parameter_THIRD);
}

static void mqtt_event(struct mqtt_connection *m, mqtt_event_t event, void *data){
    switch(event) {
    case MQTT_EVENT_CONNECTED: {
        LOG_INFO("Application has a MQTT connection\n");

        state = STATE_CONNECTED;
        break;
    }
    case MQTT_EVENT_DISCONNECTED: {
        LOG_INFO("MQTT Disconnect. Reason %u\n", *((mqtt_event_t *)data));

        state = STATE_DISCONNECTED;
        process_poll(&sensor_humidity);
        break;
    }
    case MQTT_EVENT_PUBLISH: {      //triggered when a new msg is published
        msg_ptr = data;

        if(strcmp(msg_ptr->topic, "param/humidity") == 0)
            pub_handler_humidity(msg_ptr->topic, strlen(msg_ptr->topic),msg_ptr->payload_chunk, msg_ptr->payload_length);

        break;
    }
    case MQTT_EVENT_SUBACK: {
        #if MQTT_311
            mqtt_suback_event_t *suback_event = (mqtt_suback_event_t *)data;

            if(suback_event->success) {
            LOG_INFO("Application is subscribed to topic successfully\n");
            } else {
            LOG_INFO("Application failed to subscribe to topic (ret code %x)\n", suback_event->return_code);
            }
        #else
            LOG_INFO("Application is subscribed to topic successfully\n");
        #endif
            break;
    }
    case MQTT_EVENT_UNSUBACK: {
        LOG_INFO("Application is unsubscribed to topic successfully\n");
        break;
    }
    case MQTT_EVENT_PUBACK: {
        LOG_INFO("Publishing complete.\n");
        break;
    }
    default:
        LOG_INFO("Application got a unhandled MQTT event: %i\n", event);
        break;
    }
}

static bool have_connectivity(void){
    if(uip_ds6_get_global(ADDR_PREFERRED) == NULL ||
        uip_ds6_defrt_choose() == NULL) {
        return false;
    }
    return true;
}

PROCESS_THREAD(sensor_humidity, ev, data){
    PROCESS_BEGIN();    
    LOG_INFO("MQTT sensor_humidity started\n");

    // Initialize the ClientID as MAC address
    snprintf(client_id, BUFFER_SIZE, "%02x%02x%02x%02x%02x%02x",
                        linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1],
                        linkaddr_node_addr.u8[2], linkaddr_node_addr.u8[5],
                        linkaddr_node_addr.u8[6], linkaddr_node_addr.u8[7]);

    // Broker registration					 
    mqtt_register(&conn, &sensor_humidity, client_id, mqtt_event, MAX_TCP_SEGMENT_SIZE);
    state=STATE_INIT;
                        
    // Initialize time_elapsed
    time_elapsed = 0;

    //button initialization
    btn = button_hal_get_by_index(0);

    // Initialize periodic timer to check the status 
    etimer_set(&e_timer, STATE_MACHINE_PERIODIC);

    /* Main loop */
    while(1) {

        PROCESS_YIELD();

        if((ev == PROCESS_EVENT_TIMER && data == &e_timer) || 
            ev == PROCESS_EVENT_POLL || (ev == PROCESS_EVENT_TIMER && data == &sleep_timer)){
                            
            if(state==STATE_INIT){
                if(have_connectivity()==true)  
                    state = STATE_NET_OK;
            } 
            
            if(state == STATE_NET_OK){
                // Connect to MQTT broker
                LOG_INFO("Connecting to MQTT broker!\n");
                
                memcpy(broker_address, broker_ip, strlen(broker_ip));
                
                mqtt_connect(&conn, broker_address, DEFAULT_BROKER_PORT,
                            (DEFAULT_PUBLISH_INTERVAL * 3) / CLOCK_SECOND,
                            MQTT_CLEAN_SESSION_ON);
                state = STATE_CONNECTING;
            }
            
            if(state==STATE_CONNECTED){
                // Subscribing to topic: humidity
                strcpy(sub_topic,"param/humidity");
                status = mqtt_subscribe(&conn, NULL, sub_topic, MQTT_QOS_LEVEL_0);
                LOG_INFO("Subscribing to param/humidity topic!\n");
                if(status == MQTT_STATUS_OUT_QUEUE_FULL) {
                    LOG_ERR("Tried to subscribe but command queue was full!\n");
                    PROCESS_EXIT();
                }
                
                state = STATE_SUBSCRIBED;
            }
                
            if(state == STATE_SUBSCRIBED){
                if(is_first_pub_flag == 1){
                    //publish on topic=sensor/humidity
                    ctimer_set(&sensing_timer, SENSE_PERIOD * CLOCK_SECOND, sense_callback, NULL);	
                    is_first_pub_flag = 0;	
                }
           
            
            } else if ( state == STATE_DISCONNECTED ){
            	LOG_ERR("Disconnected from MQTT broker\n");	
            	// Recover from error: try to reconnect after WAIT_FOR_RECONNECTION seconds
                state = STATE_INIT;
                etimer_set(&sleep_timer, WAIT_FOR_RECONNECTION * CLOCK_SECOND);
                continue;
            }
            
            etimer_set(&e_timer, STATE_MACHINE_PERIODIC);
        }
        else if(ev == button_hal_press_event){  //viene premuto il bottone, in base in quale intervallo sono modifico il value 
           button_pressed++;
           if(button_pressed == 1){     //umidità supera limite max dell'intervallo in cui si trova il processo 
                alarm_state = true;
                flag_over_under = 0;

           }else if(button_pressed == 2){   //umidità scende sotto limite min dell'intervallo in cui si trova il processo 
                alarm_state = true;
                flag_over_under = 1; 

            }else if(button_pressed == 3){  //torno alla normalità
                alarm_state = false;
                flag_over_under = -1;
                button_pressed = 0; 
            }
        }
    }

    PROCESS_END();
}