import paho.mqtt.client as mqtt
from threading import Thread
import mysql.connector
import json

mydb = mysql.connector.connect(
  host="localhost",
  user="root",
  password="root",
  database="RoastingMachine"
)

mycursor = mydb.cursor()

sql = "INSERT INTO dataSensed (value, type) VALUES (%s, %s)"
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
	print(msg.topic+" "+str(msg.payload.decode("utf-8","ignore")))
	json_payload = json.loads(str(msg.payload.decode("utf-8","ignore")))
	
    # insert data into database
	for key in json_payload:
		val = (int(json_payload[key]), key[:-6])		# remove _value (ex: co2_value)
		mycursor.execute(sql, val)

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