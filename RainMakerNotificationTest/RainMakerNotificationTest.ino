#include <Arduino.h>
#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"

const char *service_name = "Intrusion_ESP32";
const char *pop = "12345678";

Switch test_switch("RainMaker Test", NULL);

unsigned long startTime;
bool alertSent = false;

void sysProvEvent(arduino_event_t *sys_event)
{
  switch (sys_event->event_id)
  {
    case ARDUINO_EVENT_PROV_START:
      Serial.println("RainMaker provisioning started.");
      WiFiProv.printQR(service_name, pop, "ble");
      break;

    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      Serial.println("Wi-Fi credentials received.");
      WiFiProv.endProvision();
      break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
      Serial.println("Wi-Fi connected.");
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("ESP32 got IP address.");
      break;

    default:
      break;
  }
}

void setup()
{
  Serial.begin(115200);
  delay(1000);

  Serial.println();
  Serial.println("================================");
  Serial.println("RAINMAKER NOTIFICATION TEST");
  Serial.println("================================");

  Node my_node = RMaker.initNode("ESP32 Intrusion Detection");

  my_node.addDevice(test_switch);

  RMaker.start();

  WiFi.onEvent(sysProvEvent);

  WiFiProv.initProvision(
    NETWORK_PROV_SCHEME_BLE,
    NETWORK_PROV_SCHEME_HANDLER_FREE_BTDM
  );

  WiFiProv.beginProvision(
    NETWORK_PROV_SCHEME_BLE,
    NETWORK_PROV_SCHEME_HANDLER_FREE_BTDM,
    NETWORK_PROV_SECURITY_1,
    pop,
    service_name
  );

  startTime = millis();

  Serial.println("Waiting for RainMaker connection...");
}

void loop()
{
  // Give RainMaker time to connect to the cloud.
  if (!alertSent &&
      WiFi.status() == WL_CONNECTED &&
      millis() - startTime > 5000)
  {
    Serial.println();
    Serial.println("Sending TEST ALERT...");

    esp_rmaker_raise_alert("Test notification from ESP32");

    Serial.println("TEST ALERT SENT.");

    alertSent = true;
  }

  // Keep the test alive for 20 seconds.
  if (millis() - startTime >= 20000)
  {
    Serial.println();
    Serial.println("================================");
    Serial.println("NOTIFICATION TEST COMPLETE");
    Serial.println("================================");

    while (true)
    {
      delay(1000);
    }
  }

  delay(100);
}