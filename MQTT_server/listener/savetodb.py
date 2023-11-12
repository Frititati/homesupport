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

        self.beyyer()

    def beyyer(self):
        try:
            self.mqtt_client = MyMQTT(
                self.mqtt_client_id, self.mqtt_broker, MQTT_PORT, self
            )

            self.sql_client = psycopg2.connect(
                host=self.sql_host,
                port=SQL_PORT,
                database=self.sql_database,
                user=self.sql_user,
                password=self.sql_password,
            )

            sql = """
            CREATE TABLE IF NOT EXISTS messages (
                id SERIAL PRIMARY KEY,
                topic TEXT NOT NULL,
                payload TEXT NOT NULL,
                created_at INTEGER NOT NULL
            );
            CREATE TABLE IF NOT EXISTS temperature_readings (
                id SERIAL PRIMARY KEY,
                device_id TEXT NOT NULL,
                temperature NUMERIC NOT NULL,
                humidity NUMERIC,
                heat_index NUMERIC,
                ts timestamptz DEFAULT now()
            );
            """
            cursor = self.sql_client.cursor()
            cursor.execute(sql)
            cursor.close()
        except Exception as e:
            print(f"Reconnection Error: {str(e)}")

    def startOperation(self):
        self.mqtt_client.start()
        time.sleep(1)
        self.mqtt_client.mySubscribe(self.mqtt_topic)

    def notify(self, topic, payload):
        print(f"At '{topic}' received payload: {payload}")

        try:
            payload_parsed = str(payload.decode("utf-8")).replace("'", '"')
            data = json.loads(payload_parsed)
            device_id = data["id"]
            temperature = data["temperature"]
            humidity = data.get("humidity", None)
            heat_index = data.get("heat_index", None)

            print(f"dev {device_id}; temp {temperature}; humi {humidity}; heat {heat_index}")

            sql = "INSERT INTO temperature_readings (device_id, temperature, humidity, heat_index) VALUES (%s, %s, %s, %s)"
            cursor = self.sql_client.cursor()
            cursor.execute(sql, (device_id, temperature, humidity, heat_index))
            self.sql_client.commit()
            cursor.close()
        except json.JSONDecodeError as e:
            print(f"Failed to parse JSON payload: {str(e)}")
        except KeyError as e:
            print(f"Missing key in payload: {str(e)}")


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

    # Keep the script running in order to keep on waiting
    while True:
        time.sleep(10)
