
//https://github.com/RobTillaart/ADS1X15

#include <AsyncMqttClient.h>
#include <Wire.h>
#include <WiFi.h>
#include<PubSubClient.h>

#include "ADS1X15.h"
//#include "Adafruit_ADS1X15.h"

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}

// Wifi Credentials.
const char* ssid = "FiOS-6KACV";//"FiOS-6KACV"; //Besio3DPrinter
const char* password = "rad3coffin4025rows";//"rad3coffin4025rows";
unsigned long lastMsg = 0;

#define MSG_BUFFER_SIZE  (50)

WiFiClient espClient;
PubSubClient client(espClient);

// public broker for now.
const char* mqtt_server = "test.mosquitto.org";//"test.mosquitto.org"; // ESP- 192.168.1.162

char msg[MSG_BUFFER_SIZE];
int value = 0;

// Define publisher route.
const char* MQTT_PUB_VOLT0 = "neuroPort/adc1115/voltage0/Device1";
const char* MQTT_PUB_VOLT1 = "neuroPort/adc1115/voltage1/Device1";
const char* MQTT_PUB_VOLT2 = "neuroPort/adc1115/voltage2/Device1";
const char* MQTT_PUB_VOLT3 = "neuroPort/adc1115/voltage3/Device1";

ADS1115 ADS(0x48);


void setup_wifi() {

//  delay(10);
  // We start by connecting to a WiFi network.
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload ,unsigned int length ) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
 for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  
  Serial.println();

}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(1000);
    }
  }
}

void setup() 
{
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);  
  Serial.println(__FILE__);
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);
  ADS.begin();
  ADS.setMode(0);
}

void loop() 
{
    ADS.setGain(0);
  // Convet Analog to Digital.
  int16_t val_0 = ADS.readADC(0);  
  int16_t val_1 = ADS.readADC(1);  
  int16_t val_2 = ADS.readADC(2);  
  int16_t val_3 = ADS.readADC(3);  
  float f = ADS.toVoltage(1);  // voltage factor
  float v0 = val_0 * f;
  float v1 = val_1 * f;
  float v2 = val_2 * f;
  float v3 = val_3 * f; 
  Serial.print("\tAnalog0: "); Serial.print(val_0); Serial.print('\t'); Serial.println(v0, 3);
//  Serial.print("\tAnalog1: "); Serial.print(val_1); Serial.print('\t'); Serial.println(v1, 3);
//  Serial.print("\tAnalog2: "); Serial.print(val_2); Serial.print('\t'); Serial.println(v2 * f, 3);
//  Serial.print("\tAnalog3: "); Serial.print(val_3); Serial.print('\t'); Serial.println(v3 * f, 3);
//  Serial.println();
  delay(10);
  // handel reconnection.
  if(!client.connected()){
    reconnect();
  }
  client.loop();
 // Get timpstamp.
  unsigned long now = millis();
  
  if (now - lastMsg > 2000) {
    lastMsg = now;
    Serial.print("Publish message: V0");
    Serial.println(v0);
    client.publish(MQTT_PUB_VOLT0, String(v0).c_str());
    Serial.print("Publish message: V1");
    Serial.println(v1);
    client.publish(MQTT_PUB_VOLT1, String(v1).c_str());
    client.publish(MQTT_PUB_VOLT2, String(v2).c_str());
    client.publish(MQTT_PUB_VOLT3, String(v3).c_str());

  }

}
