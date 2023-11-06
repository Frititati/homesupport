from sub.MyMQTT import *
import paho.mqtt.client as PahoMQTT
import json
import time
from datetime import datetime


class tempSub:
    def __init__(self, clientID, broker, topic):
        self.clientID = clientID
        self.broker = broker
        self.topic = topic
        self.client = MyMQTT(self.clientID, self.broker, 1883, self)
        self.last_msg = ""

    def startOperation(self):
        # Begin operation
        self.client.start()
        time.sleep(3)
        self.client.mySubscribe(self.topic)

    def notify(self, topic, payload):
        self.msg = payload
        print(self.msg)


if __name__ == "__main__":
    topic_name = "topic/test/load"

    led = tempSub("cell_client", "filipi.local", topic_name)

    led.startOperation()

    # Keep the script running in order to keep on waiting
    while True:
        time.sleep(10)
