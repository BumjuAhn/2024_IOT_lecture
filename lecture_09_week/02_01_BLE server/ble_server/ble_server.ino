#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

// UUID 정의
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

bool deviceConnected = false;
unsigned long previousMillis = 0; // 이전 시간 저장용
const long interval = 3000; // 3초 간격

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic;

// 서버 콜백 클래스 정의
class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected");
  }
};

// 특성 콜백 클래스 정의
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // std::string을 String으로 변환하여 사용
    String rxValue = String(pCharacteristic->getValue().c_str());
    if (rxValue.length() > 0) {
      Serial.print("Received Value: ");
      Serial.println(rxValue); // 수신된 문자열을 시리얼 모니터에 출력
    }
  }
};

void setup() {
  Serial.begin(115200);
  Serial.println("Starting BLE work!");

  BLEDevice::init("BOmBzOO");

  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ |
                      BLECharacteristic::PROPERTY_WRITE |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );

  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->setValue("Hello World says Neil");

  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // iPhone 연결 문제 해결
  pAdvertising->setMinPreferred(0x12);
  BLEDevice::startAdvertising();
  Serial.println("Characteristic defined! Now you can read it in your phone!");
}

void loop() {
  // 연결된 상태에서만 "OK" 메시지를 3초마다 전송
  if (deviceConnected) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      pCharacteristic->setValue("OK");
      pCharacteristic->notify(); // "OK" 메시지를 전송
      Serial.println("Sent: OK");
    }
  }
  delay(10);
}
