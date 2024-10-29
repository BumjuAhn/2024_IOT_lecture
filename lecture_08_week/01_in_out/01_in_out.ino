#include <WiFi.h>
#include <PubSubClient.h>

// WiFi 및 MQTT 설정
const char* ssid = "your_wifi_ssid";            // WiFi SSID
const char* password = "your_wifi_password";     // WiFi 비밀번호
const char* mqttBroker = "your_mqtt_broker_ip";  // MQTT 브로커 주소
const char* mqttTopic = "test/topic";            // 발행할 MQTT 주제
const char* mqttSubscriptionTopic = "test/subscription"; // 구독할 MQTT 주제

// MQTT 클라이언트 객체
WiFiClient espClient;
PubSubClient client(espClient);

// 메시지 발행 주기
unsigned long lastMsgTime = 0;
const unsigned long publishInterval = 5000;  // 5초

// WiFi 연결
void connectToWiFi() {
  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

// MQTT 연결
void connectToMQTT() {
  client.setServer(mqttBroker, 1883);
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32_Client")) { // 클라이언트 이름 지정
      Serial.println("Connected to MQTT");

      // 연결 후 주제를 구독
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

  Serial.print("Message: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void setup() {
  Serial.begin(115200);
  connectToWiFi();
  client.setCallback(callback);  // 콜백 함수 설정
  connectToMQTT();
}

void loop() {
  // MQTT 연결 확인 및 재연결
  if (!client.connected()) {
    connectToMQTT();
  }
  client.loop();

  // 주기적으로 메시지 발행
  if (millis() - lastMsgTime >= publishInterval) {
    lastMsgTime = millis();
    String message = "Hello from ESP32!";
    client.publish(mqttTopic, message.c_str());  // 메시지 발행
    Serial.println("Published: " + message);
  }
}
