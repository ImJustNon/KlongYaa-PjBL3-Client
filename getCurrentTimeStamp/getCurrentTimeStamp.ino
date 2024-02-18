#include <TimeLib.h>
#include <WiFi.h>
#include <NTPClient.h>

const char *ssid = "NOthiNg";
const char *password = "trytoguess";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

void setup() {
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

  // Wait for time synchronization to complete
  while (time(nullptr) < 1000000000) {
    delay(1000);
    Serial.println("Waiting for time synchronization...");
  }

  Serial.println("Time synchronized!");

  Serial.println("Current timestamp (seconds since 1970-01-01 00:00:00):");
}

void loop() {
  time_t now = time(nullptr); // Get the current time

  // Convert time_t to String
  String timestampString = String(now);

  Serial.println(timestampString);

  // Update the NTP client
  timeClient.update();

  delay(1000); // Delay for 1 second (1000 milliseconds)
}
