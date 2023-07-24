#include "my_library.h"

//MQTT varaibles
const int oledLib = 1;
WiFiClient wifiClient;
MQTTClient mqttClient;

// Relay PIN
extern const int RELAYPIN = 2;  // Digital pin for relay control

const char mqttBroker[63] = "public.cloud.shiftr.io";  //"192.168.1.204";

int myRand;
char mqttClienName[31] = "client_59999_Gxxxx5678";
int intervalMQTT = 0;
long nmrMqttMesages = 0;
String mqttStringMessage;
char topicPub[3][61] = { "" };
char topicPubInfo[61] = "";
char topicSub[61] = "";

char tmpBuffer[64];
const char gNumber[15] = "Gxxxx5418";  //Replace with your last digits of G-number
String Motor_Status = "";
void Mqtt_Initialize(void) {
  sprintf(topicPubInfo, "%s%s%s", "/gmu/ece508/team06/", gNumber, "/Smart_Lawn_Irrigation_system");
  sprintf(topicPub[0], "%s%s%s", "/gmu/ece508/team06/", gNumber, "/Temperature");
  sprintf(topicPub[1], "%s%s%s", "/gmu/ece508/team06/", gNumber, "/SoilMoisture");
  sprintf(topicPub[2], "%s%s%s", "/gmu/ece508/team06/", gNumber, "/RainPrediction");
  sprintf(topicSub, "%s%s%s", "/gmu/ece508/team06/", gNumber, "/MotorStatus");

  //Generate Random MQTT ClientID
  myRand = random(0, 9999);
  sprintf(mqttClienName, "client_%04d_%s", myRand, gNumber);
  mqttClient.begin(mqttBroker, wifiClient);
  mqttClient.onMessage(messageReceived);
}
void Loop_Mqtt(JSONVar myJsonDoc, String oledline[9]) {
  mqttClient.loop();
  if (!mqttClient.connected()) {
    connectMqtt(mqttClienName);
  }

  intervalMQTT++;
  if (intervalMQTT >= 2) {
    intervalMQTT = 0;
    nmrMqttMesages++;

    myJsonDoc["class"] = String("ece508");
    myJsonDoc["team"] = String("06");
    myJsonDoc["Gnumber"] = String("Gxxx5418");
    myJsonDoc["device"] = String("nano33iot");
    convDDHHMMSS(millis() / 1000, tmpBuffer);
    myJsonDoc["uptime"] = String(tmpBuffer);
    getWiFiRSSI(tmpBuffer);
    myJsonDoc["rssi"] = String(tmpBuffer);

    myJsonDoc["msgnum"] = nmrMqttMesages;
    getMacWifiShield(tmpBuffer);
    myJsonDoc["Temperature"] = oledline[2];
    myJsonDoc["Soilmoisture"] = oledline[3];
    myJsonDoc["Rain"] = oledline[4];
    myJsonDoc["MotorStatus"] = String(Motor_Status);
    myJsonDoc["Date"] = oledline[8];

    //Jon Info publish
    mqttStringMessage = JSON.stringify(myJsonDoc);
    mqttClient.publish(topicPubInfo, mqttStringMessage);
    Serial.println(String(topicPubInfo) + " : " + String(mqttStringMessage));

    for (int i = 0; i < 3; i++) {
      mqttClient.publish(topicPub[i], oledline[2 + i]);
      Serial.println(String(topicPub[i]) + " : " + oledline[2 + i]);
    }
  }
  if (Motor_Status != "") {
    oledline[7] = Motor_Status;
  }
}

void connectMqtt(char *mqttClienName) {
  // Serial.println("connectMqtt: Checking WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    //  Serial.print(".");
    delay(1000);
  }
  //Serial.println("connectMqtt: WiFi Ok...");

  //Serial.println("connectMqtt: Checking MQTT...");
  while (!mqttClient.connect(mqttClienName, "public", "public")) {
    Serial.print(".");
    delay(1000);
  }
  //Serial.println("connectMqtt: MQTT Ok...");

  Serial.println("Subscribed: " + String(topicSub));
  mqttClient.subscribe(topicSub);
}

void messageReceived(String &topic, String &payload) {
  Serial.println("incoming: " + topic + " - " + payload);
  /*if (payload == "start") {
    Motor_Status = "ON: watering";
    digitalWrite(RELAYPIN, LOW);  // Deactivate the relay (turn it ON)
  } else if (payload == "stop") {
    Motor_Status = "OFF";
    digitalWrite(RELAYPIN, HIGH);  // Deactivate the relay (turn it OFF)
  } else {
    Serial.println("Subscribe: " + topic + " - " + payload + "INVALID command");
  }*/
}