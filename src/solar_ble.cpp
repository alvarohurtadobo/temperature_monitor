/**
 * Solar BLE - Monitor CAYAPA + Bluetooth Low Energy
 * solar.cpp + BLE (envío de au16data con adcValue).
 * Sin WebSockets. Device: ESP32_DELTA_01
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>
#include <NimBLEDevice.h>

// WiFi (variables de entorno en PlatformIO)
#ifndef WIFI_SSID
#define WIFI_SSID "SSID_NAME"
#endif
#ifndef WIFI_PASS
#define WIFI_PASS "PASSWORD"
#endif
#ifndef SERVER_URL
#define SERVER_URL "http://192.168.1.100:3000/api/store-json"
#endif

static const char* ssid = WIFI_SSID;
static const char* password = WIFI_PASS;
static const char* serverURL = SERVER_URL;

// BLE (igual que main.cpp)
#define DEVICE_NAME "ESP32_DELTA_01"
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHAR_UUID_NOTIFY "87654321-4321-4321-4321-ba0987654321"

static NimBLEServer* pServer = nullptr;
static NimBLECharacteristic* pCharacteristic = nullptr;

class MyServerCallbacks : public NimBLEServerCallbacks {
  void onDisconnect(NimBLEServer* server, NimBLEConnInfo& connInfo, int reason) override {
    (void)connInfo;
    (void)reason;
    Serial.println("Cliente BLE desconectado, reiniciando advertising...");
    server->startAdvertising();
  }
};

// au16data formato igual que main.cpp (adcValue en los tres primeros)
static uint16_t au16data[16] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0xFFFF
};

// Pines
#define ADC_PIN 36
#define LED_NEGATIVO 32
#define LED_0_10 33
#define LED_10_20 26

static LiquidCrystal_I2C lcd(0x27, 16, 2);

static unsigned long lastRequestTime = 0;
static const unsigned long requestInterval = 5000;
static int counter = 0;

static void controlarLEDs(float energy);
static void mostrarEnLCD(int adcValue, float energy);
static void enviarDatosAlServidor(float energy);

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(LED_NEGATIVO, OUTPUT);
  pinMode(LED_0_10, OUTPUT);
  pinMode(LED_10_20, OUTPUT);

  digitalWrite(LED_NEGATIVO, LOW);
  digitalWrite(LED_0_10, LOW);
  digitalWrite(LED_10_20, LOW);

  analogReadResolution(12);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");

  // BLE (solo BLE, sin WebSockets)
  NimBLEDevice::init(DEVICE_NAME);
  pServer = NimBLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  NimBLEService* pService = pServer->createService(SERVICE_UUID);

  pCharacteristic = pService->createCharacteristic(
    CHAR_UUID_NOTIFY, NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);

  pCharacteristic->setValue("Hello DELTA");
  pService->start();

  NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->start();
  Serial.println("BLE iniciado: " DEVICE_NAME);

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Conectando a WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("Conectado! IP: ");
  Serial.println(WiFi.localIP());

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP: ");
  lcd.print(WiFi.localIP());
  delay(2000);
  lcd.clear();
}

void loop() {
  counter++;

  int adcValue = analogRead(ADC_PIN);
  float energy = -10.0f + (adcValue / 4095.0f) * 30.0f;

  // au16data: adcValue en los tres primeros valores (formato exacto main.cpp)
  au16data[0] = static_cast<uint16_t>(adcValue);
  au16data[1] = static_cast<uint16_t>(adcValue);
  au16data[2] = static_cast<uint16_t>(adcValue);

  controlarLEDs(energy);
  mostrarEnLCD(adcValue, energy);

  // HTTP cada 5 s
  unsigned long currentTime = millis();
  if (currentTime - lastRequestTime >= requestInterval) {
    enviarDatosAlServidor(energy);
    lastRequestTime = currentTime;
  }

  // BLE: formato "[v0,v1,v2,...,v15]" como main.cpp, cada ~1.2 s
  if (counter % 8 == 0) {
    String serializedData = "[";
    for (int i = 0; i < 16; i++) {
      serializedData += String(au16data[i]);
      if (i < 15) {
        serializedData += ",";
      }
    }
    serializedData += "]";

    pCharacteristic->setValue(
      reinterpret_cast<uint8_t*>(const_cast<char*>(serializedData.c_str())),
      serializedData.length());
    pCharacteristic->notify();
  }

  delay(150);
}

static void controlarLEDs(float energy) {
  digitalWrite(LED_NEGATIVO, LOW);
  digitalWrite(LED_0_10, LOW);
  digitalWrite(LED_10_20, LOW);

  if (energy < 0) {
    digitalWrite(LED_NEGATIVO, HIGH);
  } else if (energy >= 0 && energy < 10) {
    digitalWrite(LED_0_10, HIGH);
  } else if (energy >= 10 && energy <= 20) {
    digitalWrite(LED_10_20, HIGH);
  }
}

static void mostrarEnLCD(int adcValue, float energy) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Energy: ");
  lcd.print(adcValue);
  lcd.print(" kWH");

  lcd.setCursor(0, 1);
  lcd.print("CAYAPA: ");
  lcd.print(energy, 2);
}

static void enviarDatosAlServidor(float energy) {
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi desconectado. Intentando reconectar...");
    WiFi.begin(ssid, password);
    return;
  }

  HTTPClient http;
  http.begin(serverURL);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<200> doc;
  doc["device_id"] = "cayapa-001";
  doc["energy"] = energy;
  doc["timestamp"] = millis() / 1000;

  String jsonString;
  serializeJson(doc, jsonString);

  Serial.print("Enviando datos: ");
  Serial.println(jsonString);

  int httpResponseCode = http.POST(jsonString);

  if (httpResponseCode > 0) {
    Serial.print("Respuesta HTTP: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error en POST: ");
    Serial.println(httpResponseCode);
  }

  http.end();
}
