#include <SPI.h>
#include <Wire.h>
#include <WiFiNINA.h>
#include "DHT.h"
#include <SimpleDHT.h>
#include <Arduino_LSM6DS3.h>
#include <time.h>
#include <MQTT.h>
#include <stdio.h>
#include <Scheduler.h>

long current_time, previous_time;
char temperature[64];
char humid[64];
char PIRBuffer[64];

const char *wifi_name = "sweethome";
const char *password = "foxpointe9210";

// DHT22 variables
SimpleDHT22 dht22(5);
float Humid_data = 0.0;
float Temp_data = 0.0;
int errDHT22 = SimpleDHTErrSuccess;

// PIR sensor variables
int PIR_Interrupt_pin = 2;
long TotalDetections = 0;
volatile int detect = 0;
volatile int state = 0;
volatile long ActiveInterval = 0;
volatile long startActiveInterval = 0;

WiFiClient wifiClient;
MQTTClient mqttClient;
const char mqttBroker[63] = "192.168.1.204";  // To connect the MQTT broker for testing purpose configured the local system IP
char mqttServerName[31] = "RTOS_Baby_Monitor";
int intervalMQTT = 0;
long totalMqttMesages = 0;
String mqttdataPublish;
char topicPub1[61] = "";
char topicPub2[61] = "";
char topicPub3[61] = "";

bool babyAwake = false;            // Variable to track if the baby is awake
unsigned long lastMotionTime = 0;  // Variable to store the time of the last motion detection

void setup() {
  Serial.begin(115200);

  randomSeed(analogRead(A7));
  sprintf(topicPub1, "%s%s%s", "RTOS_PRoject/", "Temp/", "RoomTemperature");
  sprintf(topicPub2, "%s%s%s", "RTOS_PRoject/", "dht22/", "RoomHumidity");
  sprintf(topicPub3, "%s%s%s", "RTOS_PRoject/", "PIR/", "BabyMotion");

  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  int statusWiFi = WL_IDLE_STATUS;
  while (statusWiFi != WL_CONNECTED) {
    Serial.print("Attempting to connect to WiFi: ");
    statusWiFi = WiFi.begin(wifi_name, password);
    delay(250);
  }

  //Attached the interrupt to PIR senso PIN
  pinMode(PIR_Interrupt_pin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(PIR_Interrupt_pin), detectPIR, CHANGE);

  mqttClient.begin(mqttBroker, wifiClient);
  
  // Add the task to the scheduler
  Scheduler.startLoop(ReadTemperature, 10000);
}

void loop() {
  mqttClient.loop();
  BabyMotion();  // checking if Baby is wake up.
  delay(500);
  Scheduler.yield();
}

void ReadTemperature() {
  if (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    connectMqtt(mqttServerName);
  } else {
    errDHT22 = dht22.read2(&Temp_data, &Humid_data, NULL);
    if (errDHT22 == 0) {
      Temp_data = 1.8 * Temp_data + 32;
      sprintf(temperature, "Temp: %.1f F", Temp_data);
      sprintf(humid, "Humid RH%%: %.1f", Humid_data);
    } else {
      Serial.print("Temp:  Sensor ERROR!");
    }
    mqttdataPublish = String(temperature);
    mqttClient.publish(topicPub1, mqttdataPublish);

    mqttdataPublish = String(humid);
    mqttClient.publish(topicPub2, mqttdataPublish);
  }
  Scheduler.yield();
  delay(3000);
}


/*
*  Called by attachInterrupt in which the normal flow of program execution can be temporarily halted to respond to an event
*  such as a change in the state of a PIR_Interrupt_pin. is this sets the detection high then Publish the Baby motion
*/
void detectPIR() {
  detect = 1;
  state = digitalRead(PIR_Interrupt_pin);
  if (state == 1) {
    ActiveInterval = -1;
    startActiveInterval = millis();
  } else {
    ActiveInterval = millis() - startActiveInterval;
  }
}

//Function to monitor Baby Sleep action
void BabyMotion() {

  if (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    connectMqtt(mqttServerName);
  } else {
    if (detect == 1 && state == 1) {
      detect = 0;
      TotalDetections++;
    } else {
      // If the baby was sleeping before and wakes up, publish the wakeup message
      if (state == 1) {
        babyAwake = true;
        sprintf(PIRBuffer, "*** Baby wakeup ***");
        mqttdataPublish = String(PIRBuffer);
        mqttClient.publish(topicPub3, mqttdataPublish);
        Serial.print("Pin D2: " + String(TotalDetections) + ":" + String(state) + " " + String(ActiveInterval) + "ms");
      } else if (babyAwake) {
        // If the baby was awake before and goes back to sleep, publish the sleeping message
        babyAwake = false;
        sprintf(PIRBuffer, "*** Baby Sleeping ***");
        mqttdataPublish = String(PIRBuffer);
        mqttClient.publish(topicPub3, mqttdataPublish);
        Serial.print("Pin D2: " + String(TotalDetections) + ":" + String(state) + " " + String(ActiveInterval) + "ms");
      }
    }
  }
}

/*
* MQTT connect and establish the server client mechanism between Ardunio and ESP32
*/
void connectMqtt(char *mqttServerName) {
  Serial.println("connectMqtt: Checking WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("connectMqtt: WiFi Ok...");

  Serial.println("connectMqtt: Checking MQTT...");
  while (!mqttClient.connect(mqttServerName, "public", "public")) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("connectMqtt: MQTT Ok...");
}
