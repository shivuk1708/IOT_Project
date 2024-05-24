/**************************************************************************
 Class: ECE508 Summer 2023
 Student Name: Shivakumar Nyamagoud 
 Gnumber: Gxxxx5418  
 Date: 06/27/2023
 Project: Final Project Team 06 
 Description: Smart Lawn irrigation system using Ardunio Nano IoT 33 for more reference read the paper
 Issues: 
**************************************************************************/

#include "my_library.h"

const char gNumber[15] = "G5418";         //Replace with your last digits of G-number
const char* wifi_ssid = "sweethome";      //Replace with your WiFi SSID
const char* wifi_pass = "foxpointe9210";  //Replace with your WiFi password

unsigned long currMillis, prevMillis;
char lcdBuffer[64];
String oledline[9];
unsigned long wifiEpoch = 0;

// Relay PIN
const int RELAYPIN = 2;  // Digital pin for relay control

JSONVar myJsonDoc;

void setup() {
  Serial.begin(115200);

  // Relay PIN SET
  pinMode(RELAYPIN, OUTPUT);     // Set the relay pin as an output
  digitalWrite(RELAYPIN, HIGH);  // Deactivate the relay (turn it OFF)

  //Initialize OLDE screen
  startOLEDnano33IoT_Ascii();

  oledline[1] = "G5418 FinalProject";
  for (int jj = 2; jj <= 8; jj++) { oledline[jj] = ""; }

  // check for the WiFi module:
  oledline[2] = "Wifi status...";
  displayTextOLED_Ascii(oledline);
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    while (true)
      ;
  }

  // attempting to connect to Wifi network:
  oledline[2] = "Connecting to Wifi...";
  displayTextOLED_Ascii(oledline);

  prevMillis = millis();
  int statusWiFi = WL_IDLE_STATUS;
  while (statusWiFi != WL_CONNECTED) {
    Serial.print("Attempting to connect to WiFi: ");
    statusWiFi = WiFi.begin(wifi_ssid, wifi_pass);
    delay(250);
  }
  currMillis = millis();
  sprintf(lcdBuffer, "Connected in %d sec", (currMillis - prevMillis) / 1000);
  oledline[2] = lcdBuffer;
  displayTextOLED_Ascii(oledline);

  // MQTT initilize
  Mqtt_Initialize();

  for (int jj = 2; jj <= 8; jj++) { oledline[jj] = ""; }

  prevMillis = millis();  //initial start time
}

void loop() {

  currMillis = millis();
  // digitalWrite(RELAYPIN, HIGH);  // Activate the relay (turn it on)
  // delay(5000);                   // Wait for 2 seconds
  // digitalWrite(RELAYPIN, LOW);   // Deactivate the relay (turn it off)
  // delay(5000);

  if (currMillis - prevMillis >= 1000)  //Display every second
  {
    prevMillis = currMillis;

    // Get the Today's date and time
    wifiEpoch = WiFi.getTime();
    convCurrentTimeET(wifiEpoch, lcdBuffer);
    oledline[8] = String(lcdBuffer) + " E";

    String info = "";
    Get_Weather_Info(oledline);
    Get_Soil_Moisture(oledline);

    Loop_Mqtt(myJsonDoc, oledline);
    
    displayTextOLED_Ascii(oledline);
  }
}
