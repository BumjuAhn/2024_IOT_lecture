#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>          // https://github.com/tzapu/WiFiManager
#include <PubSubClient.h>         // MQTT library

// WebServer setup
ESP8266WebServer server(80);

// MQTT setup
WiFiClient espClient;
PubSubClient client(espClient);
void callback(char* topic, byte* payload, unsigned int length);
String mqttServer = "";
int mqttPort = 1883;
String mqttInTopic = "";
String mqttOutTopic = "";

unsigned long lastMqttPublishTime = 0;

// HTML form for MQTT configuration page
const char* htmlForm = R"(
  <form action="/mqtt" method="post">
    MQTT Server: <input type="text" name="server"><br>
    MQTT Port: <input type="text" name="port" value="1883"><br>
    Input Topic: <input type="text" name="intopic"><br>
    Output Topic: <input type="text" name="outtopic"><br>
    <input type="submit" value="Save">
  </form>
)";

void setup() {
  Serial.begin(115200);

  // Connect to WiFi using WiFiManager
  WiFiManager wifiManager;
  String apName = "BOmBzOO-" + String(ESP.getChipId(), HEX);
  wifiManager.autoConnect(apName.c_str());
  Serial.println("WiFi connected");

  // Initialize web server
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlForm);
  });

  server.on("/mqtt", HTTP_POST, []() {
    mqttServer = server.arg("server");
    mqttPort = server.arg("port").toInt();
    mqttInTopic = server.arg("intopic");
    mqttOutTopic = server.arg("outtopic");

    server.send(200, "text/plain", "MQTT settings saved. Please restart the ESP.");
    Serial.println("MQTT settings saved.");
    Serial.println("Server: " + mqttServer);
    Serial.println("Port: " + String(mqttPort));
    Serial.println("Input Topic: " + mqttInTopic);
    Serial.println("Output Topic: " + mqttOutTopic);
  });

  server.begin();
  Serial.println("Web server started");
}

void loop() {
  server.handleClient();

  if (mqttServer != "" && !client.connected()) {
    connectToMQTT();
  }

  if (client.connected()) {
    client.loop();

    // Publish "MQTT connected" every 5 seconds
    // Print published message
    if (millis() - lastMqttPublishTime >= 5000) {
      client.publish(mqttOutTopic.c_str(), "MQTT connected");
      // Serial.println("Published to topic: " + mqttOutTopic + " - MQTT connected");
      // Serial.println("Published: MQTT connected");
      lastMqttPublishTime = millis();
    }
  }
}

void connectToMQTT() {
  client.setServer(mqttServer.c_str(), mqttPort);
  client.setCallback(callback);
  Serial.print("Connecting to MQTT...");

  if (client.connect("ESP8266Client")) {
    Serial.println("MQTT connected");
    client.subscribe(mqttInTopic.c_str());
    Serial.println("Subscribed to input topic: " + mqttInTopic);
  } else {
    Serial.print("MQTT connection failed, rc=");
    Serial.print(client.state());
    delay(5000);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String message;
  for (unsigned int i = 0; i < length; i++) {
    message += (char)payload[i];
  }
  Serial.println("Message received on topic: " + String(topic));
  Serial.println("Message: " + message);

  // Print received message
  if (String(topic) == mqttInTopic) {
    Serial.println("Received message: " + message);
  }
}
