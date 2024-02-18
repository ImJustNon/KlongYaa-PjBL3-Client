// ========================= import libs zone =========================
#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <ESPAsyncWebServer.h>
// ========================= import libs zone =========================

// ========================= Just Stupid variables =========================
bool startAlert = true;
// ========================= Just Stupid variables =========================

// ========================= output config zone =========================
const int ledChannelPins[] = {5, 17, 16, 4, 2, 15};
int ledChannelPinsLength = sizeof(ledChannelPins) / sizeof(ledChannelPins[0]);
const int buzzerPin = 13;
const int stopSwitchPin = 33;
const int buzzerAlertCount = 10;
// ========================= output config zone =========================

// ========================= request path config zone =========================
const String postCheckTime = "http://45.141.27.36:6060/api/bypass/non/checktime";
const String postRemoveTime = "http://45.141.27.36:6060/api/bypass/non/removetime";
// ========================= request path config zone =========================

// ========================= config zone =========================
const String boxUniqueId = "test-912108";
const String secretApiKey = "there_is_no_key_lol";
const char* ssid = "NOthiNg";
const char* password = "trytoguess";
unsigned long lastTime = 0;
unsigned long timerDelay = 1000;
String jsonBuffer;
// ========================= config zone =========================

// ========================= setup libs zone =========================
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");
AsyncWebServer server(80);
// ========================= setup libs zone =========================
// ========================= code zone =========================
void setup() {
  Serial.begin(115200);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, HIGH); // set buzzer to off mode
  pinMode(stopSwitchPin, INPUT_PULLUP);
  for(int i = 0; i < ledChannelPinsLength; i++){
    pinMode(ledChannelPins[i], OUTPUT);
  }
  connectToWifi();
  syncTime();
}

void loop(){
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() == WL_CONNECTED) { 
      checkAlertTime();
    }
    else {
      Serial.println("WiFi Disconnected");
      connectToWifi();
      syncTime();
    }
    lastTime = millis();
    startAlert = true;
  }
}



void checkAlertTime(){
  String currentTime = getCurrentTime();
  String postJsonData = "{\"boxId\":\"" + boxUniqueId + "\", \"currentTime\": \"" + currentTime + "\" }";
  String checkTimeData = httpPOSTRequest(postCheckTime.c_str(), postJsonData.c_str());
  // Serial.println(checkTimeData);
  JSONVar checkTimeDataObjectPost = JSON.parse(checkTimeData);
  if (JSON.typeof(checkTimeDataObjectPost) == "undefined"){
    Serial.println("Parsing checkTimeData input failed!");
    return;
  }
  Serial.println(checkTimeDataObjectPost);
  String alert = checkTimeDataObjectPost["data"]["alert"];
  String alertId = checkTimeDataObjectPost["data"]["alertId"];
  JSONVar ledArray = checkTimeDataObjectPost["data"]["ledChannel"];
  int ledArraySize = checkTimeDataObjectPost["data"]["ledSize"];
  if(alert == "true"){
    // led on
    for(int ii = 0; ii < ledArray.length(); ii++){
      int selectLed = ledArray[ii];
      digitalWrite(ledChannelPins[selectLed - 1], HIGH);
    }
    // buzzer alert
    while(startAlert){
      digitalWrite(buzzerPin, LOW);
      delay(250);
      digitalWrite(buzzerPin, HIGH);
      delay(500);
      int stopSwitchStatus = digitalRead(stopSwitchPin);
      if(stopSwitchStatus == 0){
        startAlert = false;
        removeAlertTime(alertId);      
      }
    }

    int stopSwitchStatus2 = digitalRead(stopSwitchPin);
    Serial.println(stopSwitchStatus2);
    // wait for release switch then off led
    while(stopSwitchStatus2 == 0){
      stopSwitchStatus2 = digitalRead(stopSwitchPin);
    }
    for(int ii = 0; ii < ledArray.length(); ii++){
      int selectLed = ledArray[ii];
      digitalWrite(ledChannelPins[selectLed - 1], LOW);
    }
  }
}

void removeAlertTime(String alertId){
  String postJsonData = "{\"boxId\":\"" + boxUniqueId + "\", \"secretApiKey\": \"" + secretApiKey + "\", \"alertId\": \"" + alertId + "\" }";
  String removeTimeData = httpPOSTRequest(postRemoveTime.c_str(), postJsonData.c_str());
  // Serial.println(removeTimeData);
  JSONVar removeTimeDataObjectPost = JSON.parse(removeTimeData);
  if (JSON.typeof(removeTimeDataObjectPost) == "undefined"){
    Serial.println("Parsing checkTimeData input failed!");
    return;
  }
  Serial.println(removeTimeDataObjectPost);
}

// ========================= Fuction Zone =========================
// ========================= connect to wifi function =========================
void connectToWifi(){
  while (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(ssid, password);
    for(int i = 0; i < 15; i++){
      if(WiFi.status() == WL_CONNECTED){
        break;
      }
      Serial.print("Trying to connect... (");
      Serial.print(i);
      Serial.println(")");
      delay(1000);
    }
  }
  

  Serial.println("");
  Serial.println("**************************************");
  Serial.print("Connected to : ");
  Serial.println(ssid);
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("**************************************");
}
// ========================= get Current Time function =========================
String getCurrentTime(){
  timeClient.update();
  time_t now = time(nullptr);
  String timestampString = String(now); 
  return timestampString + "000"; // add 000 for what??? i dont know LOL. 
}
// ========================= sync time function =========================
void syncTime(){
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 1000000000) {
    delay(1000);
    Serial.println("Waiting for time synchronization...");
  }
  Serial.println("");
  Serial.println("**************************************");
  Serial.println("Time synchronized");
  Serial.println("Current timestamp (seconds since 1970-01-01 00:00:00):");
  Serial.println("**************************************");
}
// ========================= get request function =========================
String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
  http.setTimeout(30000);
  http.begin(client, serverName);
  int httpResponseCode = http.GET();
  String payload = "{}";
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return payload;
}
// ========================= post request function =========================
String httpPOSTRequest(const char* serverName, const char* postData) {
  WiFiClient client;
  HTTPClient http;
  http.setTimeout(30000);
  http.begin(client, serverName);
  http.addHeader("Content-Type", "application/json");
  int httpResponseCode = http.POST(postData);
  String payload = "{}";
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  http.end();
  return payload;
}
