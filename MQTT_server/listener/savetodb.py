import paho.mqtt.client as mqtt
import psycopg2
from time import time, sleep
import sched

MQTT_HOST = "filipi.local"
MQTT_PORT = 1883
MQTT_CLIENT_ID = 'client_reader_test'
TOPIC = 'topic/test/a'

# PostgreSQL connection parameters
PG_HOST = 'filipi.local'
PG_PORT = '5432'
PG_DATABASE = 'your_postgresql_database'
PG_USER = 'your_postgresql_user'
PG_PASSWORD = 'your_postgresql_password'

# Reconnection interval in seconds (10 minutes)
RECONNECT_INTERVAL = 600

def on_connect(mqtt_client, user_data, flags, conn_result):
    mqtt_client.subscribe(TOPIC)

def on_message(mqtt_client, user_data, message):
    payload = message.payload.decode('utf-8')

    print(f"pay {payload}")

    db_conn = user_data['db_conn']
    sql = 'INSERT INTO messages (topic, payload, created_at) VALUES (%s, %s, %s)'
    cursor = db_conn.cursor()
    cursor.execute(sql, (message.topic, payload, int(time())))
    db_conn.commit()
    cursor.close()

def reconnect_mqtt(mqtt_client):
    try:
        mqtt_client.reconnect()
    except Exception as e:
        print(f"MQTT Reconnection Error: {str(e)}")

def reconnect_postgres(pg_conn):
    try:
        pg_conn.close()
        pg_conn = psycopg2.connect(
            host=PG_HOST,
            port=PG_PORT,
            database=PG_DATABASE,
            user=PG_USER,
            password=PG_PASSWORD
        )
        return pg_conn
    except Exception as e:
        print(f"PostgreSQL Reconnection Error: {str(e)}")
        return pg_conn

def main():
    # Connect to PostgreSQL
    pg_conn = psycopg2.connect(
        host=PG_HOST,
        port=PG_PORT,
        database=PG_DATABASE,
        user=PG_USER,
        password=PG_PASSWORD
    )

    # Create the messages table if it doesn't exist
    sql = """
    CREATE TABLE IF NOT EXISTS messages (
        id SERIAL PRIMARY KEY,
        topic TEXT NOT NULL,
        payload TEXT NOT NULL,
        created_at INTEGER NOT NULL
    )
    """
    cursor = pg_conn.cursor()
    cursor.execute(sql)
    cursor.close()

    mqtt_client = mqtt.Client(MQTT_CLIENT_ID)
    mqtt_client.user_data_set({'db_conn': pg_conn})

    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    mqtt_client.connect(MQTT_HOST, MQTT_PORT)
    mqtt_client.loop_start()  # Start MQTT loop in the background

    # Initialize the scheduler
    s = sched.scheduler(time, sleep)

    while True:
        try:
            s.enter(RECONNECT_INTERVAL, 1, reconnect_mqtt, (mqtt_client,))
            pg_conn = reconnect_postgres(pg_conn)
            s.enter(RECONNECT_INTERVAL, 1, reconnect_postgres, (pg_conn,))
            s.run()
        except KeyboardInterrupt:
            break

if __name__ == "__main__":
    main()
