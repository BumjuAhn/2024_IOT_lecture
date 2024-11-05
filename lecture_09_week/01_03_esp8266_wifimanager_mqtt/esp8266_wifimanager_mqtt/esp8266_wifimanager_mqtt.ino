#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

const int gpio0Reset = D2;              // Reset button GPIO
const int ledPin = LED_BUILTIN;         // LED pin

// MQTT server configuration
const char* mqtt_server = "ai.doowon.ac.kr"; 
const int mqtt_port = 1883;
const char* mqtt_topic = "/iot/bombzoo/out";  // MQTT topic

WiFiClient espClient;
PubSubClient mqttClient(espClient);     // MQTT client instance

unsigned long lastPublishTime = 0;      // Last message publish time

// Function to handle MQTT connection
void connectToMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Connect to MQTT without user credentials
    if (mqttClient.connect("ESP8266Client")) {
      Serial.println("connected");
      mqttClient.subscribe(mqtt_topic);  // Subscribe to the topic if needed
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

// Function to handle Wi-Fi reset button
void handleWiFiReset() {
  if (digitalRead(gpio0Reset) == LOW) {
    WiFiManager wifiManager;
    wifiManager.resetSettings();        // Reset Wi-Fi settings
    digitalWrite(ledPin, LOW);          // Turn LED on to indicate reset
    delay(1000);
    digitalWrite(ledPin, HIGH);         // Turn LED off after delay
  }
}

// Function to initialize Wi-Fi connection
void setupWiFi() {
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");
  Serial.println("Connected to WiFi.");
  digitalWrite(ledPin, LOW);            // Indicate Wi-Fi connection
  delay(2000);
  digitalWrite(ledPin, HIGH);
}

// Function to publish MQTT message at intervals
void publishMQTTMessage() {
  unsigned long currentMillis = millis();
  if (currentMillis - lastPublishTime >= 5000) { // Publish every 5 seconds
    mqttClient.publish(mqtt_topic, "Hello from ESP8266");
    Serial.println("Message published to MQTT.");
    lastPublishTime = currentMillis;  // Update last publish time
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(gpio0Reset, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  handleWiFiReset();                   // Check if reset button is pressed
  setupWiFi();                         // Initialize Wi-Fi connection
  
  mqttClient.setServer(mqtt_server, mqtt_port);  // Set up MQTT server
}

void loop() {
  // Ensure MQTT connection is active
  if (!mqttClient.connected()) {
    connectToMQTT();
  }
  mqttClient.loop();

  // Publish MQTT message if 5 seconds have passed
  publishMQTTMessage();
}
