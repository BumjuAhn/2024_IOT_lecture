#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <EEPROM.h>

const char* ap_ssid = "ESP8266_OpenAP";  // 개방형 AP 모드 SSID
AsyncWebServer server(80);

// EEPROM에 저장할 크기 설정
#define EEPROM_SIZE 96
#define SSID_ADDR 0
#define PASS_ADDR 32

String ssid = "";
String password = "";

// EEPROM에서 Wi-Fi 설정을 읽는 함수
void loadWiFiConfig() {
  char ssidArr[32];
  char passArr[64];
  EEPROM.begin(EEPROM_SIZE);

  for (int i = 0; i < 32; i++) {
    ssidArr[i] = EEPROM.read(SSID_ADDR + i);
  }
  for (int i = 0; i < 64; i++) {
    passArr[i] = EEPROM.read(PASS_ADDR + i);
  }

  ssid = String(ssidArr);
  password = String(passArr);

  ssid.trim();
  password.trim();
  EEPROM.end();
}

// Wi-Fi 설정을 EEPROM에 저장하는 함수
void saveWiFiConfig(const char* ssid, const char* password) {
  EEPROM.begin(EEPROM_SIZE);

  for (int i = 0; i < 32; i++) {
    EEPROM.write(SSID_ADDR + i, (i < strlen(ssid)) ? ssid[i] : 0);
  }
  for (int i = 0; i < 64; i++) {
    EEPROM.write(PASS_ADDR + i, (i < strlen(password)) ? password[i] : 0);
  }

  EEPROM.commit();
  EEPROM.end();
}

// 저장된 정보로 Wi-Fi에 연결하는 함수
void connectToWiFi(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");

  int attempt = 0;
  while (WiFi.status() != WL_CONNECTED && attempt < 10) {
    delay(1000);
    Serial.print(".");
    attempt++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nConnected to WiFi!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nFailed to connect to WiFi.");
  }
}

void setup() {
  Serial.begin(115200);
  EEPROM.begin(EEPROM_SIZE);

  // 저장된 Wi-Fi 설정을 로드
  loadWiFiConfig();

  // 저장된 SSID와 비밀번호가 있다면 자동으로 연결 시도
  if (ssid.length() > 0) {
    connectToWiFi(ssid.c_str(), password.c_str());
  }

  // 연결되지 않은 경우 AP 모드 시작
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.softAP(ap_ssid);
    Serial.println("Open AP Mode Started");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    // 웹 페이지 설정
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      String html = "<html><body><h2>WiFi setup</h2><form action='/connect' method='post'>"
                    "SSID: <input type='text' name='ssid'><br>"
                    "Password: <input type='password' name='password'><br>"
                    "<input type='submit' value='Connect'></form></body></html>";
      request->send(200, "text/html", html);
    });

    // Wi-Fi 연결 요청을 처리하는 부분
    server.on("/connect", HTTP_POST, [](AsyncWebServerRequest *request){
      if (request->hasParam("ssid", true) && request->hasParam("password", true)) {
        ssid = request->getParam("ssid", true)->value();
        password = request->getParam("password", true)->value();
        saveWiFiConfig(ssid.c_str(), password.c_str());

        request->send(200, "text/html", "WiFi 설정이 저장되었습니다. ESP8266을 재부팅하세요.");
        delay(2000);  // 웹 페이지에서 메시지를 볼 수 있도록 대기
        ESP.restart();
      } else {
        request->send(200, "text/html", "SSID 또는 Password가 입력되지 않았습니다.");
      }
    });

    // 웹 서버 시작
    server.begin();
  }
}

void loop() {
  // 연결 상태 확인 주기 (5초)
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("WiFi Connected.");
  } else {
    Serial.println("WiFi Not Connected.");
  }
  delay(5000);
}
