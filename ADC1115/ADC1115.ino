//
//    FILE: ADS_read.ino
//  AUTHOR: Rob.Tillaart
// VERSION: 0.2.1
// PURPOSE: read analog inputs - straightforward.
//

// test
// connect 1 potmeter per port.
//
// GND ---[   x   ]------ 5V
//            |
//
// measure at x (connect to AIN0).
//
//https://github.com/RobTillaart/ADS1X15

#include <AsyncMqttClient.h>

#include <Wire.h>
#include <WiFi.h>
#include<PubSubClient.h>

#include "ADS1X15.h"

extern "C" {
  #include "freertos/FreeRTOS.h"
  #include "freertos/timers.h"
}
#define WIFI_SSID "FiOS-6KACV"
#define WIFI_PASSWORD "rad3coffin4025rows"

#define MQTT_HOST IPAddress(192, 168, 1, 88) // 192.168.1.88
#define MQTT_PORT 1883
const char* broker = "mqtt.dioty.co"; // Change this.

// Define publisher route
const char* MQTT_PUB_VOLT0 = "neuroPort/adc1115/voltage0";
const char* MQTT_PUB_VOLT1 = "neuroPort/adc1115/voltage1";
const char* MQTT_PUB_VOLT2 = "neuroPort/adc1115/voltage2";
const char* MQTT_PUB_VOLT3 = "neuroPort/adc1115/voltage3";

ADS1115 ADS(0x48);


WiFiClient espClinet;
PubSubClient client(espClinet);

AsyncMqttClient mqttClient;
TimerHandle_t mqttReconnectTimer;
TimerHandle_t wifiReconnectTimer;

//Publishes reading every milisecond.
unsigned long previousMillis = 0;  // Stores last time temperature was published
const long interval = 2000;       // Interval at which to publish sensor readings every 10s



// MQTT functions:

void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}


void reconnect(){
  while(!client.connected()){
    Serial.println("\nConnecting to");
    Serial.println(MQTT_HOST);
    // Init server connection.
    if(client.connect("wah", brokerUser,brokerPass)){
        Serial.println("\nConnected to");
        Serial.println(MQTT_HOST);
      }
      else{
        Serial.println("\nTrying to connect");
        delay(5000);
        }
    
    
    }
  }

 //Connects tiny pico to MQTT Broker.
void connectToMqtt() {
  Serial.println("Connecting to MQTT…");
  mqttClient.connect();
 }

 void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
  Serial.println("Disconnected from MQTT.");
  if (WiFi.isConnected()) { 
    xTimerStart(mqttReconnectTimer, 0);
  }
}
void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}

 void WiFiEvent(WiFiEvent_t event) {
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch(event) {
    case SYSTEM_EVENT_STA_GOT_IP:
      Serial.println("WiFi connected");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
      connectToMqtt();
      break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
      Serial.println("WiFi lost connection");
      xTimerStop(mqttReconnectTimer, 0);
      xTimerStart(wifiReconnectTimer, 0);
      break;
  }
}

  
void setup() 
{
  Serial.begin(115200);
    
  //mqttReconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToMqtt));
  //wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void*)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));
  WiFi.onEvent(WiFiEvent);
  connectToWifi();
  client.setServer(MQTT_HOST,1883);

  /*
  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  //mqttClient.onSubscribe(onMqttSubscribe);
  //mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(broker, MQTT_PORT); // for cloud
  mqttClient.setCredentials(brokerUser, brokerPass);
  */
  
  Serial.println(__FILE__);
  Serial.print("ADS1X15_LIB_VERSION: ");
  Serial.println(ADS1X15_LIB_VERSION);

  ADS.begin();
}

void loop() 
{
  if(!client.connected()){
    reconnect();
  }
 client.loop();
 unsigned long currentMillis = millis();
 
  ADS.setGain(0);

  int16_t val_0 = ADS.readADC(0);  
  int16_t val_1 = ADS.readADC(1);  
  int16_t val_2 = ADS.readADC(2);  
  int16_t val_3 = ADS.readADC(3);  

  float f = ADS.toVoltage(1);  // voltage factor


  Serial.print("\tAnalog0: "); Serial.print(val_0); Serial.print('\t'); Serial.println(val_0 * f, 3);
  Serial.print("\tAnalog1: "); Serial.print(val_1); Serial.print('\t'); Serial.println(val_1 * f, 3);
  Serial.print("\tAnalog2: "); Serial.print(val_2); Serial.print('\t'); Serial.println(val_2 * f, 3);
  Serial.print("\tAnalog3: "); Serial.print(val_3); Serial.print('\t'); Serial.println(val_3 * f, 3);
  Serial.println();
  delay(1000);

   if (currentMillis - previousMillis >= interval) {
        previousMillis = currentMillis;
        client.publish(MQTT_PUB_VOLT0, String(val_0 * f).c_str());
        client.publish( MQTT_PUB_VOLT1 , String(val_1 * f).c_str()); 
        client.publish(MQTT_PUB_VOLT2, String(val_2 * f).c_str());  
        client.publish(MQTT_PUB_VOLT3, String(val_3 * f).c_str());  

    Serial.printf("Publishing on topic %s at QoS 1, packetId: ", MQTT_PUB_VOLT0);
    //Serial.println(packetIdPub2);
    Serial.printf("Message: V0: %.2f /n", String(val_0 * f));

    Serial.printf("Publishing on topic %s at QoS 1, packetId: ", MQTT_PUB_VOLT1);
    //Serial.println(packetIdPub3);
    Serial.printf("Message: V1: %.2f /n", String(val_1 * f).c_str());
    
    Serial.printf("Publishing on topic %s at QoS 1, packetId: ", MQTT_PUB_VOLT2);
    Serial.printf("Message: V2: %.2f /n", String(val_2 * f).c_str());

    Serial.printf("Publishing on topic %s at QoS 1, packetId: ", MQTT_PUB_VOLT3);
    Serial.printf("Message: V2: %.2f /n", String(val_3 * f).c_str());

    //Serial.println(packetIdPub4);
    Serial.printf("Mac Address ");
    Serial.println(WiFi.macAddress());
  } 
}

// -- END OF FILE --
