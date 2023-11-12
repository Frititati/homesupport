from sub.MyMQTT import *
import psycopg2
import json
import time

MQTT_PORT = 1883
SQL_PORT = 5432
HOST = "192.168.0.45"

# Reconnection interval in seconds (10 minutes)
RECONNECT_INTERVAL = 600

class SaveClass:
    def __init__(
        self,
        mqtt_client_id,
        mqtt_broker,
        mqtt_topic,
        sql_host,
        sql_database,
        sql_user,
        sql_password,
    ):
        self.mqtt_client_id = mqtt_client_id
        self.mqtt_broker = mqtt_broker
        self.mqtt_topic = mqtt_topic
        self.sql_host = sql_host
        self.sql_database = sql_database
        self.sql_user = sql_user
        self.sql_password = sql_password
        self.sql_client = None
        self.mqtt_client = None
        self.last_mqtt_connect_time = 0
        self.last_sql_connect_time = 0

    def connect_mqtt(self):
        try:
            self.mqtt_client = MyMQTT(
                self.mqtt_client_id, self.mqtt_broker, MQTT_PORT, self
            )
            self.mqtt_client.start()
            time.sleep(0.2)
            self.mqtt_client.mySubscribe(self.mqtt_topic)
            self.last_mqtt_connect_time = time.time()
            return True
        except Exception as e:
            print(f"MQTT Connection Error: {str(e)}")
            return False

    def connect_sql(self):
        try:
            self.sql_client = psycopg2.connect(
                host=self.sql_host,
                port=SQL_PORT,
                database=self.sql_database,
                user=self.sql_user,
                password=self.sql_password,
            )
            cursor = self.sql_client.cursor()
            cursor.close()
            self.last_sql_connect_time = time.time()
            return True
        except Exception as e:
            print(f"SQL Connection Error: {str(e)}")
            return False

    def startOperation(self):
        if not self.connect_mqtt() or not self.connect_sql():
            return

        while True:
            current_time = time.time()
            
            # Check if it's time to reconnect MQTT
            if current_time - self.last_mqtt_connect_time >= RECONNECT_INTERVAL:
                self.mqtt_client.stop()
                if self.connect_mqtt():
                    print("Reconnected MQTT")

            # Check if it's time to reconnect SQL
            if current_time - self.last_sql_connect_time >= RECONNECT_INTERVAL:
                self.sql_client.close()
                if self.connect_sql():
                    print("Reconnected SQL")

            time.sleep(10)

    def notify(self, topic, payload):
        print(f"At '{topic}' received payload: {payload}")
        # Handle MQTT notifications as before

if __name__ == "__main__":
    save_client = SaveClass(
        "boh",
        HOST,
        "topic/sensor/temperature",
        HOST,
        "production",
        "postgres",
        "KPqMh8Xg8KCnjqvSDnt3",
    )

    save_client.startOperation()
