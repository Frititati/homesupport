import json
import paho.mqtt.client as PahoMQTT

"""
General MQTT client
--------------------------------------------------------------------------
The class defined here implements a general MQTT client (not specific to 
publisher/subscriber).
The attribute 'notifier' specifies whether the object is a publisher or 
subscriber
--------------------------------------------------------------------------
"""

class MyMQTT:
    """
    MyMQTT
    --------------------------------------------------------------------------
    Used to create MQTT clients (both publishers and subscribers).
    --------------------------------------------------------------------------
    Passed parameters:
    - clientID: name of the client - must be unique for the chosen server
    - broker: server url
    - port: port number
    - notifier:
    --------------------------------------------------------------------------
    Attributes:
    - broker: server url
    - port: port number
    - notifier: class object having a method 'notify()' called in the method
      'myOnMessageReceived'
        - notify(topic, payload) - must be present in the subscriber class
    - clientID: client name
    - _topic: string of the considered topic
    - _isSubscriber: bool represeting what kind of client is this object ()
    - _paho_mqtt: PahoMQTT.Client() object
        - _paho_mqtt.on_connect: method used to respond to a connection
        - _paho_mqtt.on_message: method used to respond to a new message in 
          the considered topic(s)
    --------------------------------------------------------------------------
    Methods:
    - myOnConnect: used to notify when connection with the server succeedes
    - myOnMessageReceived: used to notify upon message reception - it calls 
      the 'notify' method of the class defined in attribute 'notifier'
    - myPublish: places the passed message in the specified topic
    - mySubscribe: subscribes the client to the given topic (if present) and 
      switches the _isSubscriber flag to True
    - start: used to initiate the connection to the specified broker, at the
      specified port; then the loop is initiated (allows the client to listen 
      to messages)
    - unsubscribe: unsubscribes the client from the topic (attribute 'topic')
      and sets -isSubscriber to False
    - stop: calls 'unsubscribe' and then stops the loop, before disconnecting 
      from the broker
    --------------------------------------------------------------------------
    """
    def __init__(self, clientID, broker, port, notifier):
        self.broker = broker
        self.port = port
        self.notifier = notifier
        self.clientID = clientID
        self._topic = ""
        self._isSubscriber = False
        # create an instance of paho.mqtt.client
        self._paho_mqtt = PahoMQTT.Client(clientID, True)
        # register the callback
        self._paho_mqtt.on_connect = self.myOnConnect
        self._paho_mqtt.on_message = self.myOnMessageReceived

    def myOnConnect(self, paho_mqtt, userdata, flags, rc):
        print("Connected to %s with result code: %d" % (self.broker, rc))

    def myOnMessageReceived(self, paho_mqtt, userdata, msg):
        # A new message is received
        self.notifier.notify(msg.topic, msg.payload)

    def myPublish(self, topic, msg):
        # msg is a Python dict
        # publish a message with a certain topic
        self._paho_mqtt.publish(topic, json.dumps(msg), 2)

    def mySubscribe(self, topic):
        # subscribe for a topic
        self._paho_mqtt.subscribe(topic, 2)
        # just to remember that it works also as a subscriber
        self._isSubscriber = True
        self._topic = topic
        print("subscribed to %s" % (topic))

    def start(self):
        # manage connection to broker
        self._paho_mqtt.connect(self.broker, self.port) # The usage of this method calls the method myOnConnect
        self._paho_mqtt.loop_start()

    def unsubscribe(self):
        if (self._isSubscriber):
            # remember to unsuscribe if it is working also as subscriber
            self._paho_mqtt.unsubscribe(self._topic)
            self._isSubscriber = False

    def stop(self):
        if (self._isSubscriber):
            # remember to unsuscribe if it is working also as subscriber
            self._paho_mqtt.unsubscribe(self._topic)

        self._paho_mqtt.loop_stop()
        self._paho_mqtt.disconnect()
