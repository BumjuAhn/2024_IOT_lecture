#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoBLE.h>
#include <ArduinoJson.h>

// Wi-Fi 및 MQTT 정보
char ssid[32];
char password[32];
char mqtt_broker[32];
char mqtt_topic_in[32];  // 수신 토픽
char mqtt_topic_out[32]; // 발행 토픽
bool isConfigured = false;
unsigned long lastMsgTime = 0; // 마지막 메시지 전송 시간
int count = 0; // 카운트 값

// Wi-Fi 및 MQTT 클라이언트 설정
WiFiClient espClient;
PubSubClient mqttClient(espClient);

// BLE 서비스 및 JSON 데이터 특성
BLEService configService("12345678-1234-5678-1234-56789abcdef0");
BLEStringCharacteristic jsonCharacteristic("abcd0001-1234-5678-1234-56789abcdef0", BLEWrite, 256); // JSON 데이터 특성

// MQTT 메시지 콜백 함수
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Wi-Fi 연결 함수
void connectWiFi() {
  Serial.print("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected!");
}

// MQTT 연결 함수
void connectMQTT() {
  mqttClient.setServer(mqtt_broker, 1883);
  mqttClient.setCallback(callback);
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("ESP32Client")) {
      Serial.println("Connected!");
      mqttClient.subscribe(mqtt_topic_in); // 수신 토픽 구독
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// JSON 데이터를 파싱하여 설정 값 업데이트
void parseJSONConfig(const char* json) {
  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, json);

  if (error) {
    Serial.print("Failed to parse JSON: ");
    Serial.println(error.c_str());
    return;
  }

  strlcpy(ssid, doc["ssid"] | "", sizeof(ssid));
  strlcpy(password, doc["password"] | "", sizeof(password));
  strlcpy(mqtt_broker, doc["mqtt_broker"] | "", sizeof(mqtt_broker));
  strlcpy(mqtt_topic_in, doc["mqtt_topic_in"] | "", sizeof(mqtt_topic_in)); // 수신 토픽 정보 저장
  strlcpy(mqtt_topic_out, doc["mqtt_topic_out"] | "", sizeof(mqtt_topic_out)); // 발행 토픽 정보 저장

  Serial.print("Parsed SSID: ");
  Serial.println(ssid);
  Serial.print("Parsed Password: ");
  Serial.println(password);
  Serial.print("Parsed MQTT Broker: ");
  Serial.println(mqtt_broker);
  Serial.print("Parsed MQTT Topic In: ");
  Serial.println(mqtt_topic_in);
  Serial.print("Parsed MQTT Topic Out: ");
  Serial.println(mqtt_topic_out);

  isConfigured = true;
}

void setup() {
  Serial.begin(115200);

  // BLE 초기화
  if (!BLE.begin()) {
    Serial.println("Starting BLE failed!");
    while (1);
  }

  // BLE 서비스 및 특성 설정
  BLE.setLocalName("ESP32_Config");
  BLE.setAdvertisedService(configService);
  configService.addCharacteristic(jsonCharacteristic);
  BLE.addService(configService);
  BLE.advertise();

  Serial.println("Waiting for BLE client to connect...");
}

void loop() {
  // BLE 연결 처리
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Connected to central: ");
    Serial.println(central.address());

    // JSON 데이터 수신 및 설정 적용
    while (central.connected()) {
      if (jsonCharacteristic.written()) {
        String jsonString = jsonCharacteristic.value();
        Serial.print("Received JSON: ");
        Serial.println(jsonString);

        // JSON 파싱 및 설정 값 업데이트
        parseJSONConfig(jsonString.c_str());
        break;
      }
    }
    central.disconnect();
  }

  // Wi-Fi 및 MQTT 연결
  if (isConfigured) {
    connectWiFi();
    connectMQTT();
    isConfigured = false;
  }

  // MQTT 클라이언트 처리
  if (mqttClient.connected()) {
    mqttClient.loop();
  }

  // **2초마다 발행 토픽에 메시지 발행**
  if (millis() - lastMsgTime > 2000) { // 2초 경과 시
    lastMsgTime = millis(); // 마지막 메시지 시간 갱신
    String countMessage = "count " + String(count++); // 카운트 메시지 생성
    mqttClient.publish(mqtt_topic_out, countMessage.c_str()); // 설정된 발행 토픽에 메시지 발행
    Serial.print("Published to ");
    Serial.print(mqtt_topic_out);
    Serial.print(": ");
    Serial.println(countMessage);
  }
}
