#include <WiFi.h>
#include <HTTPClient.h>
#include <Time.h>
#include <NTPClient.h>
#include <TimeLib.h>

// config zone
const char* ssid = "SBTVC_AR";
const char* password = "";
const char* server = "http://10.2.201.131:5050/api/alert/check";
const String boxId = "8654546865454596416565216546541674665546";

// time library setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");



void setup() {
  //setup serial baud
  Serial.begin(115200);
  delay(1000);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Set up time synchronization using NTP
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  while (time(nullptr) < 1000000000) {
    delay(1000);
    Serial.println("Waiting for time synchronization...");
  }
  Serial.println("Time synchronized!");
  Serial.println("Current timestamp (seconds since 1970-01-01 00:00:00):");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {  
    String currentTime = getCurrentTimestamp();

    Serial.println(currentTime);

    String requestJson = "{\"currentTime\": \"" + currentTime + "\", \"boxId\": \"" + boxId + "\"}";
    String response = sendJSONRequest(requestJson);
    Serial.println("Response: " + response);

  }

  delay(1000);
}

String sendJSONRequest(String jsonData) {
  WiFiClient client;
  HTTPClient http;
  http.begin(client, server);

  http.addHeader("Content-Type", "application/json");

  int httpResponseCode = http.POST(jsonData);

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

String getCurrentTimestamp(){
  // get current timestamp
  time_t now = time(nullptr);
  // Convert time_t to String
  String timestampString = String(now);

  // Update the NTP client
  timeClient.update();

  return timestampString;
}
