#include <ESP8266WiFi.h>
#include <WiFiManager.h>
#include <ESP8266WebServer.h> 

// 웹 서버 포트 80 설정
ESP8266WebServer server(80);

void setup() {
  Serial.begin(115200);
  Serial.println();

  // WiFiManager 초기화
  WiFiManager wifiManager;

  // 이전에 설정된 Wi-Fi 정보가 없을 경우 포털 열기
  if (!wifiManager.autoConnect("BOmBzOO_AP")) { // AP 이름을 "ESP8266_AP"로 설정
    Serial.println("WiFi 연결 실패!");
    delay(3000);
    ESP.reset(); // 연결 실패 시 보드 재부팅
    delay(5000);
  }

  // 연결 성공 메시지 및 IP 주소 출력
  Serial.println("WiFi 연결 성공!");
  Serial.print("IP 주소: ");
  Serial.println(WiFi.localIP());

  // 웹 서버 엔드포인트 설정
  server.on("/", handleRoot); // 기본 경로 처리
  server.begin();             // 웹 서버 시작
  Serial.println("웹 서버가 시작되었습니다.");
}

void loop() {
  server.handleClient(); // 웹 서버 클라이언트 요청 처리
}

// 웹 서버의 루트 핸들러 함수
void handleRoot() {
  String html = "<html><head><title>ESP8266 Wi-Fi Info</title></head><body>";
  html += "<h1>ESP8266 Wi-Fi Information</h1>";
  html += "<p><strong>Connected SSID:</strong> " + WiFi.SSID() + "</p>";
  html += "<p><strong>IP Address:</strong> " + WiFi.localIP().toString() + "</p>";
  html += "</body></html>";

  server.send(200, "text/html", html); // HTML 페이지 전송
}
