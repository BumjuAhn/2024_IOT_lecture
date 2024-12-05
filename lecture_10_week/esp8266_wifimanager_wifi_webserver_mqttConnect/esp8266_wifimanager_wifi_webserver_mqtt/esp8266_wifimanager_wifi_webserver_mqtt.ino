#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

// MQTT Information Variables
String mqtt_broker = "";
int mqtt_broker_port = 1883;
String mqtt_topic_out = "";
String mqtt_topic_in = "";

// WebServer initialization (port 80)
ESP8266WebServer server(80);

// WiFi and MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// Message count variable
int count = 0;

// HTML form for webpage
String htmlForm = "<html>\
<head><title>MQTT Settings</title></head>\
<body>\
<h1>Enter MQTT Information</h1>\
<form action='/submit' method='POST'>\
<label>MQTT Broker:</label><input type='text' name='broker'><br>\
<label>MQTT Broker Port:</label><input type='text' name='port'><br>\
<label>MQTT Topic Out:</label><input type='text' name='topic_out'><br>\
<label>MQTT Topic In:</label><input type='text' name='topic_in'><br>\
<input type='submit' value='Submit'>\
</form>\
</body>\
</html>";

void handleRoot() {
  server.send(200, "text/html", htmlForm);
}

void handleSubmit() {
  if (server.hasArg("broker") && server.hasArg("port") && server.hasArg("topic_out") && server.hasArg("topic_in")) {
    mqtt_broker = server.arg("broker");
    mqtt_broker_port = server.arg("port").toInt();
    mqtt_topic_out = server.arg("topic_out");
    mqtt_topic_in = server.arg("topic_in");

    Serial.println("MQTT information has been successfully set:");
    Serial.println("MQTT Broker: " + mqtt_broker);
    Serial.println("MQTT Broker Port: " + String(mqtt_broker_port));
    Serial.println("MQTT Topic Out: " + mqtt_topic_out);
    Serial.println("MQTT Topic In: " + mqtt_topic_in);

    server.send(200, "text/html", "<html><body><h2>MQTT information has been saved successfully!</h2></body></html>");

    // Set up MQTT server
    client.setServer(mqtt_broker.c_str(), mqtt_broker_port);
    client.setCallback(callback);
  } else {
    server.send(400, "text/html", "<html><body><h2>Please fill in all fields.</h2></body></html>");
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      client.subscribe(mqtt_topic_in.c_str());
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);

  WiFiManager wifiManager;
  wifiManager.autoConnect("BOmBzOO_AP");

  Serial.println("WiFi connected");

  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.begin();

  Serial.println("Web server started. Access at /");

  client.setClient(espClient);
}

void loop() {
  server.handleClient();

  if (!client.connected() && mqtt_broker != "") {
    reconnect();
  }
  client.loop();

  static unsigned long lastPublish = 0;
  if (millis() - lastPublish >= 3000) {
    lastPublish = millis();
    String message = "mqtt count " + String(count++);
    client.publish(mqtt_topic_out.c_str(), message.c_str());
    Serial.println("Published: " + message);
  }
}
