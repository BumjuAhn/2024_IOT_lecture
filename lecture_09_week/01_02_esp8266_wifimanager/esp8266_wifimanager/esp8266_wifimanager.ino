#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>       

int gpio0Reset = D2;

const int ledPin =  LED_BUILTIN;// the number of the LED pin

WiFiServer server(80);

void setup() {
  Serial.begin(9600);
  pinMode(gpio0Reset, INPUT_PULLUP);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH);

  WiFiManager wifiManager;
  
  int resetBtn = digitalRead(gpio0Reset);
  if(resetBtn == 0) {
    wifiManager.resetSettings();
    digitalWrite(ledPin, LOW);
  }
 
  wifiManager.autoConnect();
  
  Serial.println("Connected.");
  digitalWrite(ledPin, LOW);
  delay(2000);
  digitalWrite(ledPin, HIGH);
  
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

  // Prepare the response
  String s="<html>";
  s=s+"<meta name='viewport' content='width=device-width, initial-scale=1.0'/>";
  s=s+"<meta http-equiv='Content-Type' content='text/html;charset=utf-8' />";
  //s=s+"<meta http-equiv='refresh' content='5'/>";
  s=s+"<meta http-equiv='Content-Type' content='text/html;charset=utf-8' />";
  s=s+"<head></head><body>SmartConfig 접속 안녕하세요!<br>";
  s=s+"연결한 IP주소 : <a href='http://"+WiFi.localIP().toString()+"'/>"+WiFi.localIP().toString()+"</a>";
  s=s+"</body></html>";

  client.print(s);
  delay(1);
  Serial.println("Client disonnected");
}