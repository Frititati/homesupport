
#define NUMERIC_ID 1

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 14 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11

// #define LEDPIN 2

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 30       /* in seconds */

DHT dht(DHTPIN, DHTTYPE);

// WiFi PARAMETERS
const char *ssid = "Casa Chieri Sicuro 4";         // Enter your WiFi name
const char *password = "IJKs/@VOdn12R`CHKIJei@zh"; // Enter WiFi password
const int MAXWIFITRIES = 100;

// MQTT BROKER
const char *mqtt_broker = "filipi.local";
const char *topic = "topic/test/load";
const char *mqtt_username = "cell_publisher";
// const char *mqtt_password = "publisher";
const int mqtt_port = 1883;

// MQTT PARAMETERS
WiFiClient espClient;
PubSubClient client(espClient);
const int MAXMQTTTRIES = 5;

// CONNECT TO WIFI
// tries to connect, if succeeded returns true else returns false
bool connectWiFi()
{
  // conuter of number of connection attempts
  int tries = 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.print("Connecting to WiFi ");
  while (WiFi.status() != WL_CONNECTED && tries < MAXWIFITRIES)
  {
    Serial.print("t ");
    tries++;
    // 1 second
    delay(1000);
  }

  // check number of tries for return value
  if (tries < MAXWIFITRIES && WiFi.status() == WL_CONNECTED)
  {
    Serial.printf("[connected in %s]\n", String(tries));
    return true;
  }

  return false;
}

// CONNECT TO MQTT SERVER
bool connectMqtt()
{
  // connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  String client_id = "XXXX-";
  client_id += String(WiFi.macAddress());
  Serial.printf("Connecting to MQTT broker\n", client_id.c_str());
  if (client.connect(client_id.c_str()))
  {
    Serial.println("MQTT broker connected");
  }
  else
  {
    Serial.printf("Connection failed with state: %s", client.state());
    return false;
  }
  return true;
}

// DISPLAY RECEIVED MESSAGE
// function called when a message arrives in the topic
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}

bool work_part()
{
  if (!connectWiFi())
  {
    Serial.println("FAIL: WiFi connection");
    return false;
  }

  delay(500);

  if (!connectMqtt())
  {
    Serial.println("FAIL: MQTT connection");
    return false;
  }

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float hi = dht.computeHeatIndex(t, h, false);

  if (isnan(t) || isnan(h))
  {
    Serial.println(F("FAIL: read from DHT sensor"));
    return false;
  }
  Serial.print("Temperature: ");
  Serial.println(t);
  Serial.print("Humidity: ");
  Serial.println(h);
  Serial.print("Heat Index: ");
  Serial.println(hi);

  static char msg[256];

  String device_id = "esp";
  device_id += String(NUMERIC_ID);

  sprintf(msg, "{'id': '%s', 'temperature': %f, 'humidity': %f, 'heat_index': %f}", device_id, t, h, hi);

  client.publish(topic, msg);

  Serial.print("mqtt send:  ");
  Serial.println(msg);

  delay(10);

  client.disconnect();
  WiFi.disconnect();
  return true;
}

// void who_i_am()
// {
//   for (int i = 0; i < 2; i++)
//   {
//     for (int i = 0; i < NUMERIC_ID; i++)
//     {
//       delay(150);
//       digitalWrite(LEDPIN, HIGH);
//       delay(150);
//       digitalWrite(LEDPIN, LOW);
//     }
//     delay(500);
//   }
// }

void setup()
{
  Serial.begin(115200);
  // pinMode(LEDPIN, OUTPUT);

  // 1s delay
  delay(1000);
  Serial.println("");
  Serial.println("Wakeup");

  // start temperature sensor
  dht.begin();

  work_part();

  client.disconnect();
  WiFi.disconnect();
  delay(500);
  Serial.println("Going to sleep!");
  ESP.deepSleep(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("THIS WILL NEVER BE PRINTED");
}

void loop()
{
  Serial.println("NOTHING SHOULD BE CALLED HERE!");
  delay(1000);
}