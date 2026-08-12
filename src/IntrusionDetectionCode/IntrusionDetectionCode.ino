#include <Arduino.h>
#include "RMaker.h"
#include "WiFi.h"
#include "WiFiProv.h"

// =====================================================
// PIN DEFINITIONS
// =====================================================

#define TRIG_PIN    5
#define ECHO_PIN    18

#define GREEN_LED   19
#define RED_LED     21
#define BUZZER      22

// =====================================================
// SYSTEM SETTINGS
// =====================================================

// Intruder detection threshold
const float INTRUDER_DISTANCE = 10.0;   // cm

// Total program runtime
const unsigned long PROGRAM_TIME = 30000;   // 30 seconds

// Notification interval
const unsigned long ALERT_INTERVAL = 10000; // 10 seconds

// RainMaker provisioning information
const char *service_name = "Intrusion_ESP32";
const char *pop = "12345678";

// =====================================================
// RAINMAKER DEVICE
// =====================================================

Switch intrusion_device("Intrusion Detection", NULL);

// =====================================================
// VARIABLES
// =====================================================

unsigned long startTime = 0;
unsigned long lastAlertTime = 0;

bool alertSent = false;

// =====================================================
// RAINMAKER / WIFI EVENTS
// =====================================================

void sysProvEvent(arduino_event_t *sys_event)
{
  switch (sys_event->event_id)
  {
    case ARDUINO_EVENT_PROV_START:

      Serial.println();
      Serial.println("RainMaker provisioning started.");

      WiFiProv.printQR(
        service_name,
        pop,
        "ble"
      );

      break;

    case ARDUINO_EVENT_PROV_CRED_SUCCESS:

      Serial.println("Wi-Fi credentials received.");

      WiFiProv.endProvision();

      break;

    case ARDUINO_EVENT_WIFI_STA_CONNECTED:

      Serial.println("Wi-Fi connected!");

      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:

      Serial.print("ESP32 IP Address: ");
      Serial.println(WiFi.localIP());

      break;

    default:

      break;
  }
}

// =====================================================
// HC-SR04 DISTANCE MEASUREMENT
// =====================================================

float getDistance()
{
  // Ensure trigger is LOW
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send 10 µs trigger pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  // Measure echo pulse
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);

  // No echo received
  if (duration == 0)
  {
    return -1;
  }

  // Calculate distance in cm
  float distance = duration * 0.0343 / 2.0;

  return distance;
}

// =====================================================
// SEND RAINMAKER ALERT
// =====================================================

void sendIntruderAlert(float distance)
{
  if (WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Wi-Fi not connected. Alert not sent.");
    return;
  }

  char alertMessage[120];

  snprintf(
    alertMessage,
    sizeof(alertMessage),
    "INTRUDER DETECTED! Distance: %.1f cm",
    distance
  );

  Serial.println();
  Serial.println("Sending RainMaker notification...");

  esp_rmaker_raise_alert(alertMessage);

  Serial.println("RainMaker notification sent.");

  alertSent = true;
  lastAlertTime = millis();
}

// =====================================================
// SETUP
// =====================================================

void setup()
{
  Serial.begin(115200);

  delay(1000);

  startTime = millis();

  Serial.println();
  Serial.println("========================================");
  Serial.println("   ESP32 INTRUSION DETECTION SYSTEM");
  Serial.println("========================================");

  // -------------------------------------------------
  // GPIO CONFIGURATION
  // -------------------------------------------------

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  // Initial safe state
  digitalWrite(GREEN_LED, HIGH);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);

  Serial.println("Hardware initialized.");

  // -------------------------------------------------
  // RAINMAKER
  // -------------------------------------------------

  Node my_node = RMaker.initNode(
    "ESP32 Intrusion Detection System"
  );

  my_node.addDevice(intrusion_device);

  RMaker.start();

  WiFi.onEvent(sysProvEvent);

  // BLE provisioning for original ESP32
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

  Serial.println("RainMaker initialized.");
  Serial.println("Intrusion monitoring started.");
  Serial.println("----------------------------------------");
}

// =====================================================
// MAIN LOOP
// =====================================================

void loop()
{
  // =================================================
  // 30-SECOND AUTOMATIC STOP
  // =================================================

  if (millis() - startTime >= PROGRAM_TIME)
  {
    Serial.println();
    Serial.println("========================================");
    Serial.println("       30 SECOND TEST COMPLETE");
    Serial.println("       SYSTEM STOPPING");
    Serial.println("========================================");

    // Turn everything OFF
    digitalWrite(GREEN_LED, LOW);
    digitalWrite(RED_LED, LOW);
    digitalWrite(BUZZER, LOW);

    // Permanently stop the application
    while (true)
    {
      delay(1000);
    }
  }

  // =================================================
  // MEASURE DISTANCE
  // =================================================

  float distance = getDistance();

  // No valid measurement
  if (distance < 0)
  {
    Serial.println("No echo detected.");
    delay(200);
    return;
  }

  Serial.print("Distance: ");
  Serial.print(distance, 1);
  Serial.println(" cm");

  // =================================================
  // INTRUDER DETECTED
  // =================================================

  if (distance <= INTRUDER_DISTANCE)
  {
    Serial.println("!!! INTRUDER DETECTED !!!");

    // Red ON
    digitalWrite(RED_LED, HIGH);

    // Green OFF
    digitalWrite(GREEN_LED, LOW);

    // Buzzer ON
    digitalWrite(BUZZER, HIGH);

    // ------------------------------------------------
    // NOTIFICATION
    // ------------------------------------------------

    if (!alertSent)
    {
      // First notification immediately
      sendIntruderAlert(distance);
    }
    else if (millis() - lastAlertTime >= ALERT_INTERVAL)
    {
      // Another notification after 10 seconds
      sendIntruderAlert(distance);
    }
  }

  // =================================================
  // NO INTRUDER
  // =================================================

  else
  {
    Serial.println("Area clear.");

    // Green ON
    digitalWrite(GREEN_LED, HIGH);

    // Red OFF
    digitalWrite(RED_LED, LOW);

    // Buzzer OFF
    digitalWrite(BUZZER, LOW);

    // Reset notification timer/state
    // A new intrusion will immediately generate an alert.
    alertSent = false;
    lastAlertTime = 0;
  }

  delay(300);
}