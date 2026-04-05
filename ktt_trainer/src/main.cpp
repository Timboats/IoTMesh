#include <Arduino.h>
#include <WiFi.h>


const char *ssid = "Lebron";
const char *password = "jamesking";

void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("Configuring client...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }

  Serial.println(WiFi.localIP());
}

unsigned long prev_time = 0;
void lopp()
{
  unsigned long curr_time = millis();
  if(curr_time - prev_time > 500)
  {
    Serial.printf("RSSI: %ddB\n", WiFi.RSSI());
    prev_time = curr_time;
  }
}