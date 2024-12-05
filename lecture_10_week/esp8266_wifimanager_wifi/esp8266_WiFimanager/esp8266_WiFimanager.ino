#include <ESP8266WiFi.h>
#include <WiFiManager.h> // WiFiManager 라이브러리 포함

void setup() {
  Serial.begin(115200);
  Serial.println();

  // WiFiManager 초기화
  WiFiManager wifiManager;

  // 이전 설정된 Wi-Fi 정보가 있을 경우 자동으로 연결, 없으면 포털 열기
  if (!wifiManager.autoConnect("BOmBzOO_AP")) { // AP 이름 설정 (예: "ESP8266_AP")
    Serial.println("WiFi 연결 실패!");
    delay(3000);
    ESP.reset(); // 연결 실패 시 보드 재부팅
    delay(5000);
  }

  // 연결되면 IP 주소 출력
  Serial.println("WiFi 연결 성공!");
  Serial.print("IP 주소: ");
  Serial.println(WiFi.localIP());
}

void loop() {
  // Wi-Fi 연결이 성공한 후 할 작업을 여기에 추가
}
