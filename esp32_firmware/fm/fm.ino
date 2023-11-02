
#include <WiFi.h>
#include <PubSubClient.h>
#include "DHT.h"

#define DHTPIN 4     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

int ready_to_send = 1;

// WiFi PARAMETERS
const char *ssid = "Casa Chieri Sicuro 4"; // Enter your WiFi name
const char *password = "IJKs/@VOdn12R`CHKIJei@zh";  // Enter WiFi password
const int MAXWIFITRIES = 50;


// MQTT BROKER
// Using public MQTT broker
const char *mqtt_broker = "filipi.local";
const char *topic = "topic/test/load";
const char *mqtt_username = "cell_publisher";
//const char *mqtt_password = "publisher";
const int mqtt_port = 1883;


// MQTT PARAMETERS
WiFiClient espClient;
PubSubClient client(espClient);
const int MAXMQTTTRIES = 5;


// CONNECT TO WIFI
// tries to connect, if succeeded returns true else returns false
bool connectWiFi() {
  // conuter of number of connection attempts
  int tries = 0;
  // set working mode to station and
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ");
  // while (WiFi.status() == WL_IDLE_STATUS && tries < MAXWIFITRIES) {
  while (WiFi.status() != WL_CONNECTED && tries < MAXWIFITRIES) {
    Serial.print(".");
    tries++;
    delay(1000);
  }
  // check number of tries for return value
  if (tries < MAXWIFITRIES && WiFi.status() == WL_CONNECTED)
    
    return true;
  return false;
}


// CONNECT TO MQTT SERVER
bool connectMqtt() {
  //connecting to a mqtt broker
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(callback);
  String client_id = "XXXX-";
  client_id += String(WiFi.macAddress());
  Serial.printf("Connecting to MQTT broker ...\n", client_id.c_str());
  if (client.connect(client_id.c_str())) {
    Serial.println("MQTT broker connected");
  } else {
    Serial.print("Connection failed with state: ");
    Serial.println(client.state());
    return false;
  }
  return true;
}


// DISPLAY RECEIVED MESSAGE
// function called when a message arrives in the topic
void callback(char *topic, byte *payload, unsigned int length) {
  Serial.print("Message arrived in topic: ");
  Serial.println(topic);
  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char) payload[i]);
  }
  Serial.println();
  Serial.println("-----------------------");
}



//ostringstream ostream;

//StaticJsonBuffer<300> JSONbuffer;
//JsonObject& JSONencoder = JSONbuffer.createObject();

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



void setup() {

  // Set software serial baud to 115200;
  // for debug pourposes
  Serial.begin(115200);

  dht.begin();

  // if connection returns false
  if (!connectWiFi()) {
    Serial.println("WiFi connection failed");
  }

  
  // if connection returns false
  if (!connectMqtt()) {
    // (save in log?)
    Serial.println("MQTT connection failed");
  }
  
  // send_data();

  //deep sleep
  WiFi.disconnect();
  Serial.println("Going to sleep now");
  Serial.flush();    
}

void loop() {

  delay(20000);

    // if connection returns false
  if (!connectWiFi()) {
    Serial.println("WiFi connection failed");
  }

  
  // if connection returns false
  if (!connectMqtt()) {
    // (save in log?)
    Serial.println("MQTT connection failed");
  }

  // client.loop();
 
  float t = dht.readTemperature();

  if (isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    return;
  }
  Serial.print(F("Temperature: "));
  Serial.println(t);

  static char msg[256];
  
  sprintf(msg, "{'ID': %s, 'temperature': %f}", "ciao", t);
  client.publish(topic, msg);
  Serial.print("load sent:  ");
  Serial.println(msg);
  delay(10);
  
  client.disconnect();
  WiFi.disconnect();



}

