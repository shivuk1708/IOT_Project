#ifndef MY_LIBRARY_H
#define MY_LIBRARY_H

#include <stdio.h>
#include <Arduino.h>
#include <Arduino_JSON.h>
#include <WiFiNINA.h>
#include <MQTT.h>
#include <time.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <ArduinoMqttClient.h>

void startOLEDnano33IoT_Ascii(void);
void displayTextOLED_Ascii(String oledline[9]);

void convDDHHMMSS(unsigned long currSeconds, char *uptimeDDHHMMSS);
void convCurrentTimeET(unsigned long currSeconds, char *currentTimeET);

void getWiFiRSSI(char *wifiRSSI);
void getMacWifiShield(char *macWifiShield);
void getMacRouter(char *macRouter);


void Get_Weather_Info(String oledline[9]);
void Get_Rain_Info(String oledline[9]);
void Get_Soil_Moisture(String oledline[9]);
void Mqtt_Initialize(void);
void Loop_Mqtt(JSONVar myJsonDoc, String oledline[9]);

//MQTT
void connectMqtt(char *mqttClienName);
void messageReceived(String &topic, String &payload);
#endif
