import paho.mqtt.client as mqtt
from threading import Thread
import mysql.connector
import json

mydb = mysql.connector.connect(
  host="localhost",
  user="root",
  password="root1234",
  database="RoastingMachine"
)

mycursor = mydb.cursor()

sql = "INSERT INTO dataSensed (value, type, `interval`) VALUES (%s, %s, %s)"

# ------------------------------------------------------------------------------------
# The callbacks for when the client receives a CONNACK response from the server.
def on_connect_humidity(client, userdata, flags, rc):
	print("Connected with result code " + str(rc))
	client.subscribe("sensor/humidity")

def on_connect_temp_co2(client, userdata, flags, rc):
	print("Connected with result code " + str(rc))
	client.subscribe("sensor/temp_co2")

# ------------------------------------------------------------------------------------

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic + " " + str(msg.payload.decode("utf-8", "ignore")))
    
    # Decodifica del messaggio ricevuto in formato JSON
    json_payload = json.loads(str(msg.payload.decode("utf-8", "ignore")))

    if msg.topic == "sensor/humidity":
        # Gestisci il messaggio di umidità
        humidity_value = int(json_payload.get("humidity_value", 0))  # valore di umidità
        interval = int(json_payload.get("interval", 0))  # intervallo associato
        
        # Inserisci i dati nel database
        val_humidity = (humidity_value, "humidity", interval)
        mycursor.execute(sql, val_humidity)

    elif msg.topic == "sensor/temp_co2":
        # Gestisci il messaggio di temperatura e CO2
        temp_value = int(json_payload.get("temp_value", 0))  # valore di temperatura
        co2_value = int(json_payload.get("co2_value", 0))  # valore di CO2
        
        # Inserisci i dati nel database
        val_temp = (temp_value, "temperature", None)  # Nessun intervallo per temperatura
        val_co2 = (co2_value, "co2", None)  # Nessun intervallo per CO2
        
        mycursor.execute(sql, val_temp)
        mycursor.execute(sql, val_co2)

    # Conferma le modifiche nel database
    mydb.commit()


on_connect_callbacks = { 
	"humidity" : on_connect_humidity,
	"temp_co2": on_connect_temp_co2
}

#configure the mqtt client
def mqtt_client(topic):
	client = mqtt.Client()
	client.on_connect = on_connect_callbacks[topic]
	client.on_message = on_message
	client.connect("127.0.0.1", 1883, 60)   # connect to broker
	client.loop_forever()   # start the loop that waits for data


def main():
	thread_humidity = Thread(target=mqtt_client, args=("humidity",))
	thread_temp_co2= Thread(target=mqtt_client, args=("temp_co2",))
	
	# starting threads
	thread_humidity.start()
	thread_temp_co2.start()	

if __name__ == '__main__':
	main()