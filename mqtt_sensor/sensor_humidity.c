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

// Defaukt config values
#define DEFAULT_BROKER_PORT         1883
#define DEFAULT_PUBLISH_INTERVAL    (30 * CLOCK_SECOND)     //intervallo per la pubblicazione dei dati

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

#define SENSE_PERIOD 		2		// seconds
#define SENSE_PERIOD_ON_ALERT 	1		// seconds
#define NUM_PERIOD_BEFORE_SEND  30	// every 1 minute there's one pub

// Time tracking
static uint16_t time_elapsed = 0;
#define FIRST_INTERVAL  (5 * 60)   // 5 minutes
#define SECOND_INTERVAL (10 * 60)  // 10 minutes
#define THIRD_INTERVAL  (15 * 60)  // 15 minutes

static int num_period = 0;
static int is_first_pub_flag = 1;
static int interval = 0;
button_hal_button_t *btn;
/*---------------------------------------------------------------------------*/

static bool out_of_range(int value){

        if (time_elapsed <= FIRST_INTERVAL) {
            interval = 1; 
            if (value < min_humidity_parameter_FIRST) {   
                flag_over_under = 1; 
                return true; 
            }else if (value > max_humidity_parameter_FIRST){
                flag_over_under = 0; 
                return true;
            }else
                return false;
        } else if (time_elapsed > FIRST_INTERVAL && time_elapsed <= SECOND_INTERVAL) {
            interval = 2; 
            if (value < min_humidity_parameter_SECOND) {
                flag_over_under = 1; 
                return true; 
            }else if(value > max_humidity_parameter_SECOND){
                flag_over_under = 0; 
                return true; 
            }else 
                return false;
        } else if (time_elapsed > SECOND_INTERVAL && time_elapsed <= THIRD_INTERVAL) {
            interval = 3; 
            if (value < min_humidity_parameter_THIRD) {
                flag_over_under = 1; 
                return true; 
            }else if(value > max_humidity_parameter_THIRD){
                flag_over_under = 0; 
                return true; 
            }else 
                return false; 
        }else {
            // Time has exceeded all intervals, start new cycle 
            alarm_state = false;
            flag_over_uder = -1; 
            time_elapsed = 0;  // Reset the timer for the next cycle
            interval = 1;       //Reset interval 
            return false; 
        }

}

/* SENSING SIMULATION */
//simulazione lettura di un sensore di umidità
static int fake_humidity_sensing(int value){

    if(!alarm_state && out_of_range(value)){   //controllare se il bottone è stato appena premuto
        alarm_state = true; 
        return value; 
    }else if(alarm_state){
        if(flag_over_under == 1)
            return (value += 5);
        if(flag_over_under == 0)
            return (value -= 5); 
    }else if(!out_of_range(value)){
        if(interval == 1){
            return (rand() %(max_humidity_parameter_FIRST - min_humidity_parameter_FIRST)) + min_humidity_parameter_FIRST;
        }
        if(interval == 2){
            return (rand() %(max_humidity_parameter_SECOND - min_humidity_parameter_SECOND)) + min_humidity_parameter_SECOND;
        }
        if(interval == 3){
            return (rand() %(max_humidity_parameter_THIRD - min_humidity_parameter_THIRD)) + min_humidity_parameter_THIRD;
        }
    }
}

static void sense_callback(void *ptr){	
	value = fake_humidity_sensing(value);
	LOG_INFO("Humidity value detected = %d%s", value,(alarm_state)? "\t -> ALERT STATE\n":"\n");

    if(!out_of_range(value)){       //quando rientro nel range esco dall'alarm state 
        alarm_state = false; 
        flag_over_under = -1; 
    }

    if(alarm_state)
        time_elapsed += SENSE_PERIOD_ON_ALERT;
    else
        time_elapsed += SENSE_PERIOD; 


    if(num_period >= NUM_PERIOD_BEFORE_SEND){
        sprintf(pub_topic, "%s", "sensor/humidity");	
        sprintf(app_buffer, "{ \"humidity_value\": %d }", value);
        mqtt_publish(&conn, NULL, pub_topic, (uint8_t *)app_buffer,strlen(app_buffer), MQTT_QOS_LEVEL_0, MQTT_RETAIN_OFF);
        num_period = 0;
    }
    else if(alarm_state){
        sprintf(pub_topic, "%s", "sensor/humidity");	
        sprintf(app_buffer, "{ \"humidity_value\": %d }", value);
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
                if (time_elapsed <= FIRST_INTERVAL)
                    value = max_humidity_parameter_FIRST + 10; 
                else if (time_elapsed > FIRST_INTERVAL && time_elapsed <= SECOND_INTERVAL)
                    value = max_humidity_parameter_SECOND + 10;
                else if (time_elapsed > SECOND_INTERVAL && time_elapsed <= THIRD_INTERVAL) 
                    value = max_humidity_parameter_THIRD + 10;

           }else if(button_pressed == 2){   //umidità scende sotto limite min dell'intervallo in cui si trova il processo 
                if (time_elapsed <= FIRST_INTERVAL)
                    value = min_humidity_parameter_FIRST - 10; 
                else if (time_elapsed > FIRST_INTERVAL && time_elapsed <= SECOND_INTERVAL)
                    value = min_humidity_parameter_SECOND - 10;
                else if (time_elapsed > SECOND_INTERVAL && time_elapsed <= THIRD_INTERVAL)
                    value = min_humidity_parameter_THIRD - 10;

                button_pressed = 0; 
            }
        }
    }

    PROCESS_END();
}