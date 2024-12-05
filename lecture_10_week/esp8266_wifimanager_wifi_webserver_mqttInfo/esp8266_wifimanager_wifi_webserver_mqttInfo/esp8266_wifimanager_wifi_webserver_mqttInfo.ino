#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager

// MQTT 정보 변수
String mqtt_broker = "";
String mqtt_broker_port = "";
String mqtt_topic_out = "";
String mqtt_topic_in = "";

// WebServer 초기화 (포트 80)
ESP8266WebServer server(80);

// 웹페이지 폼 HTML
String htmlForm = "<html>\
<head><title>MQTT 설정</title></head>\
<body>\
<h1>MQTT Setup</h1>\
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
  // 루트 페이지 접근 시 폼 출력
  server.send(200, "text/html", htmlForm);
}

void handleSubmit() {
  // 폼 데이터 수신 및 MQTT 정보 저장
  if (server.hasArg("broker") && server.hasArg("port") && server.hasArg("topic_out") && server.hasArg("topic_in")) {
    mqtt_broker = server.arg("broker");
    mqtt_broker_port = server.arg("port");
    mqtt_topic_out = server.arg("topic_out");
    mqtt_topic_in = server.arg("topic_in");

    // 시리얼 모니터에 MQTT 정보 출력
    Serial.println("MQTT 정보가 성공적으로 설정되었습니다:");
    Serial.println("MQTT Broker: " + mqtt_broker);
    Serial.println("MQTT Broker Port: " + mqtt_broker_port);
    Serial.println("MQTT Topic Out: " + mqtt_topic_out);
    Serial.println("MQTT Topic In: " + mqtt_topic_in);

    // 사용자에게 성공 메시지 표시
    server.send(200, "text/html", "<html><body><h2>MQTT Info Submitted!</h2></body></html>");
  } else {
    // 잘못된 요청 처리
    server.send(400, "text/html", "<html><body><h2>Please Fill the form completely.</h2></body></html>");
  }
}

void setup() {
  Serial.begin(115200);

  // WiFiManager를 사용하여 Wi-Fi 설정
  WiFiManager wifiManager;
  wifiManager.autoConnect("BOmBzOO-AP");

  Serial.println("WiFi 연결 완료");

  // 웹 서버 핸들러 등록
  server.on("/", handleRoot);
  server.on("/submit", HTTP_POST, handleSubmit);
  server.begin();

  Serial.println("Web server 시작됨. /로 접근하세요.");
}

void loop() {
  // 웹 서버 실행
  server.handleClient();
}
