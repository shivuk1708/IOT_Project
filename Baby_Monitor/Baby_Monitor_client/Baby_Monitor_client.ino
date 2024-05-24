#include <stdio.h>
#include <WiFi.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <MQTT.h>

//Using I2C protocol to communicate with OLED display
#define I2C_ADDRESS 0x3C
Adafruit_SSD1306 myOled(128, 64, &Wire, -1);

//Home wifi password
const char *wifi_name = "sweethome";
const char *password = "foxpointe9210";

unsigned long current_time, privious_time;
char lcdBuffer[64];
String Display_data[9];
WiFiClient wifiClient;
MQTTClient mqttClient;
const char mqttBroker[63] = "192.168.1.204";  // To connect the MQTT broker for testing purpose configured the local system IP
char mqttClienESP32[31] = "RTOS_Client_ESP32";
char topicSub1[61] = "";
char topicSub2[61] = "";
char topicSub3[61] = "";

void setup() {
  pinMode(RGB_BUILTIN, OUTPUT);
  Serial.begin(115200);
  sprintf(topicSub1, "%s%s%s", "RTOS_PRoject/", "Temp/", "RoomTemperature");
  sprintf(topicSub2, "%s%s%s", "RTOS_PRoject/", "dht22/", "RoomHumidity");
  int mydand = 1231;
  sprintf(mqttClienESP32, "RTOS_Client");


  Wire.begin(4, 5);  //SDA, SCL
  if (!myOled.begin(SSD1306_SWITCHCAPVCC, I2C_ADDRESS)) {
    Serial.println("SSD1306 allocation failed");
  };

  Display_data[1] = "RTOS Project Client";
  int i;
  for (i = 2; i <= 8; i++) { Display_data[i] = ""; }

  WiFi.mode(WIFI_STA);
  WiFi.begin(wifi_name, password);
  Display_data[2] = "Connecting to Wifi...";
  displayTextOLED(Display_data);
  privious_time = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.println("Attempting to connect to WiFi...");
    delay(250);
  }
  current_time = millis();
  sprintf(lcdBuffer, "Connected in %d sec", (current_time - privious_time) / 1000);
  Display_data[2] = lcdBuffer;
  displayTextOLED(Display_data);

  mqttClient.begin(mqttBroker, wifiClient);
  mqttClient.onMessage(messageReceived);

  privious_time = millis();
}

void loop() {

  mqttClient.loop();

  if (!mqttClient.connected()) {
    for (int i = 2; i <= 8; i++) { Display_data[i] = ""; }
    Display_data[2] = String("Connecting to MQTT...");
    displayTextOLED(Display_data);
    connectMqtt(mqttClienESP32);
  }
  Display_data[2] = "";
  current_time = millis();

  if (current_time - privious_time > 1000) {
    privious_time = current_time;

    convDDHHMMSS(current_time / 1000, lcdBuffer);
    Display_data[8] = "Uptime: " + String(lcdBuffer);
    displayTextOLED(Display_data);
  }
}

void displayTextOLED(String Display_data[9]) {
  int i;

  myOled.clearDisplay();
  myOled.setTextSize(1);
  myOled.setTextColor(SSD1306_WHITE);
  myOled.setCursor(0, 0);

  for (i = 1; i <= 8; i++) {
    myOled.println(Display_data[i]);
  }

  myOled.display();
}

void convDDHHMMSS(unsigned long currSeconds, char *uptimeDDHHMMSS) {
  int dd, hh, mm, ss;

  ss = currSeconds;  //258320.0 2 23:45:20
  dd = (ss / 86400);
  hh = (ss - (86400 * dd)) / 3600;
  mm = (ss - (86400 * dd) - (3600 * hh)) / 60;
  ss = (ss - (86400 * dd) - (3600 * hh) - (60 * mm));

  sprintf(uptimeDDHHMMSS, "%d %02d:%02d:%02d", dd, hh, mm, ss);
};


void connectMqtt(char *mqttClienESP32) {
  Serial.println("connectMqtt: Checking WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println("connectMqtt: WiFi Ok...");

  Serial.println("connectMqtt: Checking MQTT...");
  while (!mqttClient.connect(mqttClienESP32, "public", "public")) {
    Serial.print(".");
    delay(1000);
  }

  Serial.println("connectMqtt: MQTT Ok...");
  sprintf(topicSub1, "%s%s%s", "RTOS_PRoject/", "Temp/", "RoomTemperature");
  sprintf(topicSub2, "%s%s%s", "RTOS_PRoject/", "dht22/", "RoomHumidity");
  sprintf(topicSub3, "%s%s%s", "RTOS_PRoject/", "PIR/", "BabyMotion");

  mqttClient.subscribe(topicSub1);
  mqttClient.subscribe(topicSub2);
  mqttClient.subscribe(topicSub3);
}

void messageReceived(String &topic, String &Data_received) {
  Serial.println("incoming: " + topic + " - " + Data_received);
  // print temperature and hu idity when it gets update from ardunio
  if (topic == "RTOS_PRoject/Temp/RoomTemperature") {
    Display_data[3] = Data_received;
  } else if (topic == "RTOS_PRoject/dht22/RoomHumidity") {
    Display_data[4] = Data_received;
  } else if (topic == "RTOS_PRoject/PIR/BabyMotion") {
    if (String("*** Baby Sleeping ***") != Data_received) {
      digitalWrite(RGB_BUILTIN, HIGH);
      Display_data[6] = Data_received;
      delay(100);
      digitalWrite(RGB_BUILTIN, LOW);
    } else {
      Display_data[6] = Data_received;
    }
  }
  displayTextOLED(Display_data);
}
