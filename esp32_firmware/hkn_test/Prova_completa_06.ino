/*
 * -------------------- Calibration --------------------
 * - Initialize HX711 object (scale)
 * - Assign pins
 * - scale.begin(<dt> <sck>)
 * - Call set_scale();
 * - Call tare();
 * - Place a known weight and call get_units(10) - 10 means it will take the average over 20 measurements
 * - Call set_scale() with the parameter obtained
 */


/*
 *  Scale info: HX711 is "ready", i.e., working when the "DOUT" pin is LOW
 *  --> Look at the pin to check for disconnected sensor
 */

/*
 *  Idea: save a flag in flash memory to run calibration at startup
 *  -> EEPROM library

    Prima prova funzionante: si colleg al wifi e al brocker, legge valori dalla cella di carico e invia correttamente i dati.
    Va in sleep se in in qualche funzione precedente si riscontra qualche problema

    DA PROVARE PIU ACCURATAMENTE 

 */


#include <WiFi.h>
#include <PubSubClient.h>
#include <HX711.h>
#include <EEPROM.h>
//#include <ArduinoJson.h>


int ready_to_send = 1;

// eeprom
#define EEPROM_SIZE 12

// TIMER
#include "MyTimer.h"
MyTimer process = MyTimer();
const int TIME_WAKE_UP = 30000;


// DEEP_SLEEP PARAMETERS
#define uS_TO_S_FACTOR 1000000ULL  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  4        /* Time ESP32 will go to sleep (in seconds) */
RTC_DATA_ATTR int bootCount = 0;


// WiFi PARAMETERS
const char *ssid = "The Provider"; // Enter your WiFi name
const char *password = "auguri2022";  // Enter WiFi password
const int MAXWIFITRIES = 50;


// MQTT BROKER
// Using public MQTT broker
const char *mqtt_broker = "mqtt.eclipseprojects.io";
const char *topic = "topic/test/load";
const char *mqtt_username = "cell_publisher";
//const char *mqtt_password = "publisher";
const int mqtt_port = 1883;


// MQTT PARAMETERS
WiFiClient espClient;
PubSubClient client(espClient);
const int MAXMQTTTRIES = 5;

// LOAD CELL PARAMETERS
const int LOADCELL_DOUT = 16;   // Digital output pin of the cell
const int LOADCELL_SCK = 17;    // Serial clock pin of the cell
const int GAIN = 128;           // Used to select correct HX711 port

const int LEDPIN = 23;

RTC_DATA_ATTR float peso = 500; // reference mass - grams
RTC_DATA_ATTR float cal;
RTC_DATA_ATTR float calibration;
RTC_DATA_ATTR float scale_par;
RTC_DATA_ATTR uint err_counter = 0;


RTC_DATA_ATTR float units; // Value of measurements (in grams)
RTC_DATA_ATTR float prev_units = 0;

RTC_DATA_ATTR float tare_thresh = 0.3;  // Threshold value for the tare (in grams)

RTC_DATA_ATTR int delay_time = 5000; // ms - delay time between measurements

RTC_DATA_ATTR bool iter_flag = 0;
RTC_DATA_ATTR bool calibrated = 0;

HX711 scale;



/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}


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
  String client_id = "cestino-";
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
void send_data(){
  //convert float to char 
  static char units_to_send[10];
  static char msg[256];
  
  Serial.println("UNITS:  ");
  Serial.println(units);
  dtostrf (units, 9, 2, units_to_send);
  //trasmetto al server dati la misura sui pesi se non nulli
  if ( ready_to_send ){
  Serial.println(topic);
  sprintf(msg, "{'ID': %s, 'weight': %s}", "ciao", units_to_send);
  client.publish(topic, msg);
  Serial.print("load sent:  ");
  Serial.println(units_to_send);
  delay(10);
  } else{
    // Non si verifica mai - ready_to_send = 1 (hard coded)
    Serial.print("Not ready"); 
  }
}



void setup() {

  // Set software serial baud to 115200;
  // for debug pourposes
  Serial.begin(115200);

  //EEPROM
  EEPROM.begin(EEPROM_SIZE);

  // DEEP SLEEP
  //Increment boot number and print it every reboot
  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));
  //Print the wakeup reason for ESP32
  print_wakeup_reason();
  /*
  First we configure the wake up source
  We set our ESP32 to wake up every 5 seconds
  */
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +
  " Seconds");
  
  // inizializzo timer ad oggetti
  process.set(TIME_WAKE_UP);

  // PIN MODE
  pinMode(LEDPIN, OUTPUT);


  // LOAD CELL
  scale.begin(LOADCELL_DOUT, LOADCELL_SCK, GAIN);
  
  load_calibration();


  // if connection returns false
  if (!connectWiFi()) {
    Serial.println("WiFi connection failed");
    //while(true);
    // go back to sleep
    Serial.println("Going to sleep now");
    Serial.flush(); 
    esp_deep_sleep_start();
    // (save in log?)
  }

  
  // if connection returns false
  if (!connectMqtt()) {
    // (save in log?)
    Serial.println("MQTT connection failed");
    //while(true);
    // go back to sleep
    Serial.println("Going to sleep now");
    Serial.flush(); 
    esp_deep_sleep_start();
    
  }
  
  // rest of the code

  // SEND MESSAGES
  // publish the message in the String, it returns true if succeded false if not, it can fail if message is too long
  //client.publish("topic/test/hw", "Hello world!");
  //Serial.println("HELLO WORLD!");

  measure_load();
  
  send_data();

  //deep sleep
  WiFi.disconnect();
  Serial.println("Going to sleep now");
  Serial.flush(); 
  esp_deep_sleep_start();

  // RECEIVE MESSAGES
  // used to subscribe to the topic, maybe could be created an additional topic for incoming messages (needed?)
  // client.subscribe(topic);

    
}

void loop() {

  client.loop();
 
 /* //timer che permette di far girare il programma prima di mandarlo in sleep
 if (process.check()){
   WiFi.disconnect();
   //deep sleep
   Serial.println("Going to sleep now");
   Serial.flush(); 
   esp_deep_sleep_start();
  }
  */

 

}


//function to measure load
void measure_load() {

  uint tmp_ready = 0;  // Checks for disconnections
  while (scale.is_ready() != 1) {
    delay(100);
    tmp_ready++;
    if (tmp_ready >= 30) {
      // ERROR - the adc could be disconnected (not available for 3 seconds)
    }
  }

  // Measure  
  digitalWrite(LEDPIN, HIGH);
  delay(10);
  digitalWrite(LEDPIN, LOW);
  units = scale.get_units(5); // Average value over 5 measurements
  digitalWrite(LEDPIN, HIGH);
  delay(10);
  digitalWrite(LEDPIN, LOW);

  // If the value changed by too much, it could be an error
  if (iter_flag > 0 && (units - prev_units) >= 2000) {
    // ERROR - what happened?
  }

  // If the value is low, tare
  if (units < tare_thresh && units > -tare_thresh) {
    scale.tare();
    Serial.println("Tare done");
  }

  // negative weights have no meaning
  if (units < 0) {
    units = 0;
    Serial.println("Negative weight detected");
  }

  if ((prev_units - units) >= 1.0 || (prev_units - units) <= -1.0) {
    Serial.print("Measured load: ");
    Serial.print(units);
    Serial.println(" g");
  }

  prev_units = units;
  iter_flag++;

  scale.power_down();
  //delay(delay_time);
  //scale.power_up();
}

void load_calibration() {
  if(!calibrated){
   
  Serial.print("Set up of the scale\n");

  bool ready_flg = 0;

  while (err_counter < 30 && ready_flg != 1) {
    if (scale.is_ready() == 1) {
      ready_flg = 1;
      scale.set_scale();
  
      Serial.println("Calibration: remove any object from the scale.");
      delay(7000);
      scale.tare();

      Serial.println("Place a known weight on the scale.");
      delay(7000);
      calibration = scale.get_units(20);

      Serial.print("cal: ");
      Serial.println(calibration);

      scale_par = calibration/peso; // grams

      EEPROM.writeFloat(0, scale_par);
      EEPROM.writeLong(4, scale.get_offset());
      EEPROM.commit();

      Serial.print("Calibration parameter: ");
      Serial.println(scale_par);
      

      scale.set_scale(scale_par);

      calibrated = 1;

      Serial.println("Scale ready");
    } else {
      Serial.println("...");
      delay(500);
      err_counter++;
    }

    if (err_counter > 20) {
      Serial.println("ERROR (calibrate) - Unable to reach cells!");
    }
  }
  } else {

    scale_par = EEPROM.readFloat(0);
    scale.set_scale(scale_par);
    scale.set_offset(EEPROM.readLong(4));
    Serial.print("Calibration parameter: ");
    Serial.println(scale_par);
      
  }
}