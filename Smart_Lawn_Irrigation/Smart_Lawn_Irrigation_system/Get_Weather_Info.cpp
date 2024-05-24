#include "JSONVar.h"
#include "my_library.h"
#include <string.h>

// Weather API Server details and endpoint and parameters
char server[] = "api.openweathermap.org";
char endpoint[] = "/data/2.5/weather";
char queryParams[] = "?q=Ashburn,US&APPID=dca01d5f86d3a010c7a38128af92a055";
int serverPort = 443;
WiFiSSLClient httpClient;
JSONVar RainObject;
JSONVar oledObject;
char* day[] = { "Today", "Tomorrow" };
char tempBuffer[64];
extern String Motor_Status;
volatile int rainforecatsflag = 0;
// SoilMoisture variables
unsigned int Moisture_Low = 250;
unsigned int Moisture_High = 800;
void Get_Weather_Info(String oledline[9]) {

  httpClient.connect(server, serverPort);
  httpClient.print(String("GET ") + endpoint + queryParams + " HTTP/1.1\r\n" + "Host: " + server + "\r\n" + "Connection: close\r\n\r\n");

  while (httpClient.connected()) {
    String line = httpClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Headers received");
      break;
    }
  }

  while (httpClient.available()) {
    String response = httpClient.readStringUntil('\n');
    //Serial.println(response);
    oledObject = oledObject.parse(response);

    double value = oledObject["main"]["temp"];
    if (value != 0) {
      double temp = ((float)value - 273.15) * 9 / 5 + 32;
      sprintf(tempBuffer, "Temp: %.2f F", temp);
    }

    //If currently it is raing then it updats the rain other wise display the forecast
    const char* Rain = NULL;
    Rain = oledObject["weather"][0]["main"];
    if (Rain != NULL && Rain == "rain") {
      const char* RainLevel = NULL;
      RainLevel = oledObject["weather"][0]["description"];
      String Level = String(RainLevel);
      Level.replace("rain", "");
      oledline[4] = String(Rain) + "ing: " + Level;
      Serial.println(String(Rain) + "ing: " + Level);
      rainforecatsflag = 1;
      break;
    }
  }
  oledline[2] = String(tempBuffer);

  if (oledline[4] == "") {
    Get_Rain_Info(oledline);
  }
  httpClient.stop();
}

// Weather API Server details and endpoint and parameters
char rain_server[] = "www.7timer.info";
char rain_endpoint[] = "/bin/api.pl";
char rain_requestParams[] = "?lon=-77.487&lat=39.044&product=civillight&output=json";
WiFiSSLClient rainhttpClient;

//https://www.7timer.info/bin/api.pl?lon=-77.487&lat=39.044&product=civillight&output=json

void Get_Rain_Info(String oledline[9]) {
  rainhttpClient.connect(rain_server, serverPort);
  rainhttpClient.print(String("GET ") + rain_endpoint + rain_requestParams + " HTTP/1.1\r\n" + "Host: " + rain_server + "\r\n" + "Connection: close\r\n\r\n");

  while (rainhttpClient.connected()) {
    String line = rainhttpClient.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("Rain Data received");
      break;
    }
  }

  String jsondata = "";
  while (rainhttpClient.available()) {
    String response = rainhttpClient.readString();
    jsondata += response;
  }
  Serial.println(String(jsondata));

  jsondata.replace("\n", "");  //remove '\n' from string
  jsondata.replace(" ", "");   //remove A from string
  //if any garbage before Json String remove that also
  if (jsondata.length() > 20) {
    int i = 0;
    while (1) {
      if (jsondata.charAt(i) == '{') {
        jsondata.remove(0, i);
        break;
      }
      i++;
    }
  }
  Serial.println(String(jsondata));

  // Parse the JSON string
  DynamicJsonDocument DataSeries(1024);  // Adjust the size according to your JSON data
  DeserializationError error = deserializeJson(DataSeries, jsondata);
  JsonArray jsonArray = DataSeries["dataseries"].as<JsonArray>();
  // Convert the array elements to strings and print them

  unsigned int count = 0;

  for (JsonVariant value : jsonArray) {
    String Date = value["date"].as<String>();
    String RainStatus = value["weather"].as<String>();

    // if you want to test the feature motor should not turn on if there is raining today and tomorrow
    // please uncomment below line and comment above line and test it
    //String RainStatus = "rain";

    // if the rain forecast is rain then set flah high so the motor will not turn on
    if (count == 0) {
      if (RainStatus == "rain") {
        rainforecatsflag = 1;
      } else {
        rainforecatsflag = 0;
      }
    }
    Serial.println("Date: " + Date + "-> " + RainStatus);
    oledline[4 + count] = String(day[count]) + String("-> " + RainStatus);
    if (++count == 2) {
      count = 0;
      break;
    }
  }
  rainhttpClient.stop();
}
void Get_Soil_Moisture(String oledline[9]) {
  int Moisture_Data = analogRead(A0);
  Serial.println("Moisture Data: " + String(Moisture_Data));
  int MoistureValue = map(Moisture_Data, Moisture_High, Moisture_Low, 0, 100);
  MoistureValue = constrain(MoistureValue, 0, 100);  // Ensure the percentage is within the valid range
  oledline[3] = String("Moisture:  ") + String(MoistureValue);
  // if rain forecatse is high then do not turn on motor
  if (rainforecatsflag != 1) {
    if (MoistureValue < 40) {
      // Turn ON watring motor
      Motor_Status = "Motor is ON";
      digitalWrite(2, LOW);
    } else {
      // Turn OFF watring motor after moisture value reaches > 90
      if (MoistureValue > 90) {
        Motor_Status = "Motor is OFF";
        digitalWrite(2, HIGH);
      }
    }
  }
}