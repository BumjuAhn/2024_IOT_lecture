#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include <ArduinoJson.h>

// WiFi 및 MQTT 설정
const char* ssid = "your_wifi_ssid";                // WiFi SSID
const char* password = "your_wifi_password";         // WiFi 비밀번호
const char* mqttBroker = "your_mqtt_broker_ip";      // MQTT 브로커 주소
const char* mqttPublishTopic = "sensor/data";        // 발행할 MQTT 주제
const char* mqttSubscriptionTopic = "control/output"; // 제어 명령을 받을 MQTT 주제

// MQTT 클라이언트 객체
WiFiClient espClient;
PubSubClient client(espClient);

// 센서 객체
Adafruit_AHTX0 aht;

// 출력 및 입력 핀 배열
const int outputPins[4] = {26, 27, 32, 33};
const int inputPins[4] = {16, 17, 18, 19};

// 메시지 발행 주기
unsigned long lastPublishTime = 0;
const unsigned long publishInterval = 5000;  // 5초

// WiFi 연결 함수
void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

// MQTT 연결 함수
void connectToMQTT() {
  client.setServer(mqttBroker, 1883);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_Client")) {
      Serial.println("Connected to MQTT");

      // 연결 후 제어 명령 주제를 구독
      client.subscribe(mqttSubscriptionTopic);
      Serial.println("Subscribed to topic: " + String(mqttSubscriptionTopic));
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" trying again in 5 seconds");
      delay(5000);
    }
  }
}

// MQTT 메시지 수신 시 호출되는 콜백 함수
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message received on topic: ");
  Serial.println(topic);

  // 구독한 메시지를 JSON으로 파싱
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload, length);
  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  int order = doc["order"];
  int pinIndex = doc["no"];
  bool value = doc["value"];

  // "order"가 2일 때 출력 핀 제어
  if (order == 2 && pinIndex >= 0 && pinIndex < 4) {
    digitalWrite(outputPins[pinIndex], value ? HIGH : LOW);
    Serial.print("Set output pin ");
    Serial.print(outputPins[pinIndex]);
    Serial.print(" to ");
    Serial.println(value ? "ON" : "OFF");
  }
}

// 센서 데이터를 발행하는 함수
void publishSensorData() {
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  float temperature = temp.temperature;
  float humidityValue = humidity.relative_humidity;

  StaticJsonDocument<256> doc;
  doc["type"] = 3;
  doc["email"] = "a@gmail.com";
  doc["temp"] = temperature;
  doc["humi"] = (int)humidityValue;

  JsonArray inArray = doc.createNestedArray("in");
  for (int i = 0; i < 4; i++) {
    inArray.add(digitalRead(inputPins[i]));
  }

  JsonArray outArray = doc.createNestedArray("out");
  for (int i = 0; i < 4; i++) {
    outArray.add(digitalRead(outputPins[i]));
  }

  String payload;
  serializeJson(doc, payload);
  client.publish(mqttPublishTopic, payload.c_str());
  Serial.println("Published: " + payload);
}

void setup() {
  // Serial 및 WiFi 초기화
  Serial.begin(115200);
  connectToWiFi();

  // MQTT 설정
  client.setCallback(callback);
  connectToMQTT();

  // 센서 초기화
  if (!aht.begin()) {
    Serial.println("Could not find AHT sensor!");
    while (1) delay(10);
  }

  // 출력 및 입력 핀 설정
  for (int i = 0; i < 4; i++) {
    pinMode(outputPins[i], OUTPUT);
    pinMode(inputPins[i], INPUT);
  }
}

void loop() {
  // MQTT 연결 상태 확인 및 재연결
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();

  // 주기적으로 센서 데이터 발행
  if (millis() - lastPublishTime >= publishInterval) {
    lastPublishTime = millis();
    publishSensorData();
  }
}
