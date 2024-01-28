#include <ESP8266WiFi.h>

const char* ssid = "Nama WiFi";
const char* password = "Password WiFi";

const int heatSensorPin = A0;  // Pin sensor suhu
const int buzzerPin = D1;     // Pin buzzer atau LED

void setup() {
  Serial.begin(115200);
  pinMode(heatSensorPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  connectToWiFi();
}

void loop() {
  float temperature = readTemperature();
  if (temperature > 30.0) {  // Ubah threshold sesuai kebutuhan
    activateAlarm();
  } else {
    digitalWrite(buzzerPin, LOW);
  }
  delay(1000);
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

float readTemperature() {
  int sensorValue = analogRead(heatSensorPin);
  float temperature = sensorValue * (5.0 / 1023.0) * 100.0;
  Serial.print("Temperature: ");
  Serial.println(temperature);
  return temperature;
}

void activateAlarm() {
  digitalWrite(buzzerPin, HIGH);
  Serial.println("ALERT: High Temperature Detected!");
}

