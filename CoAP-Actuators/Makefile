CONTIKI = ../../..

# Nome del progetto
CONTIKI_PROJECT = CoAP-Actuators
all: $(CONTIKI_PROJECT)

# Escludi piattaforme non supportate
PLATFORMS_EXCLUDE = sky z1

# Include moduli e risorse
MODULES_REL += ./resources
MODULES_REL += ../json_util

# Include la libreria CoAP
MODULES += $(CONTIKI_NG_APP_LAYER_DIR)/coap

# Include file di configurazione di Contiki
include $(CONTIKI)/Makefile.dir-variables
include $(CONTIKI)/Makefile.include


