#!/bin/bash

# Definisci i percorsi dei tuoi file
COAP_SERVER_JAR="coap_server/target/coap_server-1.0-SNAPSHOT.jar"
REMOTE_CONTROL_APPLICATION_JAR="remote_control_application/target/remote_control_application-1.0-SNAPSHOT.jar"
COLLECTOR_SCRIPT="MQTT_collector/collector.py"

# Funzione per avviare un comando in un nuovo terminale
start_in_new_terminal() {
    local title=$1
    local command=$2
    gnome-terminal --title="$title" -- bash -c "$command; exec bash"
}

# Avvia il server CoAP in un nuovo terminale
start_in_new_terminal "CoAP Server" "java -jar $COAP_SERVER_JAR"

# Avvia l'applicazione di controllo remoto in un nuovo terminale
start_in_new_terminal "Remote Control Application" "java -jar $REMOTE_CONTROL_APPLICATION_JAR"

# Avvia il collector in Python in un nuovo terminale
start_in_new_terminal "Collector" "python3 $COLLECTOR_SCRIPT"

start_in_new_terminal "make TARGET=nrf52840 BOARD=dongle PORT=/dev/ttyACM0 connect-router" 

echo "Tutti i servizi sono stati avviati in terminali separati."

