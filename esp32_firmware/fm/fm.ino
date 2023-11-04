
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 4 // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11

#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP 30       /* in seconds */

DHT dht(DHTPIN, DHTTYPE);

RTC_DATA_ATTR int bootCount = 0;

const char *device_id = "esp1";

// WiFi PARAMETERS
const char *ssid = "Casa Chieri Sicuro 4";         // Enter your WiFi name
const char *password = "IJKs/@VOdn12R`CHKIJei@zh"; // Enter WiFi password
const int MAXWIFITRIES = 50;

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

void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

// CONNECT TO WIFI
// tries to connect, if succeeded returns true else returns false
bool connectWiFi()
{
  // conuter of number of connection attempts
  int tries = 0;
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
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

// ostringstream ostream;

// StaticJsonBuffer<300> JSONbuffer;
// JsonObject& JSONencoder = JSONbuffer.createObject();

// function to send data from hx711
// void send_data(){
//   //convert float to char
//   static char units_to_send[10];
//   static char msg[256];

//   Serial.println("UNITS:  ");
//   Serial.println(units);
//   dtostrf (units, 9, 2, units_to_send);
//   //trasmetto al server dati la misura sui pesi se non nulli
//   if ( ready_to_send ){
//   Serial.println(topic);
//   sprintf(msg, "{'ID': %s, 'weight': %s}", "ciao", units_to_send);
//   client.publish(topic, msg);
//   Serial.print("load sent:  ");
//   Serial.println(units_to_send);
//   delay(10);
//   } else{
//     // Non si verifica mai - ready_to_send = 1 (hard coded)
//     Serial.print("Not ready");
//   }
// }

void setup()
{
  Serial.begin(115200);
  // 1s delay
  delay(1000);

  ++bootCount;
  Serial.print("Boot number: ");
  Serial.println(bootCount);

  print_wakeup_reason();

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // start temperature sensor
  dht.begin();

  workatit();

  client.disconnect();
  WiFi.disconnect();
  delay(1000);
  Serial.flush();
  esp_deep_sleep_start();
  Serial.println("THIS WILL NEVER BE PRINTED");
}

bool workatit()
{
  if (!connectWiFi())
  {
    Serial.println("FAIL: WiFi connection");
    return false;
  }

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

  sprintf(msg, "{'id': '%s', 'temperature': %f, 'humidity': %f, 'heat_index': %f}", device_id, t, h, hi);

  client.publish(topic, msg);

  Serial.print("mqtt send:  ");
  Serial.println(msg);

  delay(10);

  client.disconnect();
  WiFi.disconnect();
  return true;
}

void loop()
{
  Serial.println("NOTHING SHOULD BE CALLED HERE!");
}