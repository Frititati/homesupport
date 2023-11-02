import time
import json
import paho.mqtt.client as paho

MQTT_HOST="mqtt.eclipseprojects.io"
MQTT_PORT = 1883
MQTT_CLIENT_ID = 'client_writer_test'
# MQTT_USER = 'YOUR MQTT USER'
# MQTT_PASSWORD = 'YOUR MQTT USER PASSWORD'
TOPIC = "topic/test/load"


#define callback
def on_message_arrive(client, userdata, message):
    print("received message =",str(message.payload.decode("utf-8")))

client= paho.Client(MQTT_CLIENT_ID)
client.on_message=on_message_arrive

print(f"Connecting to broker {MQTT_HOST}")
client.connect(MQTT_HOST, MQTT_PORT)
client.loop_start() #start loop to process received messages
print("subscribing ")
client.subscribe(TOPIC)
print("publishing ")
client.publish(TOPIC,"test")

json_test_message = json.dumps(['foo', {'bar': ('baz', None, 1.0, 2)}])
client.publish(TOPIC, json_test_message)
client.disconnect()
client.loop_stop()