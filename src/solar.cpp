/**
 * Solar - Monitor de energía (CAYAPA)
 * Lee ADC, controla LEDs, muestra en LCD y envía datos JSON al servidor.
 * Basado en legacy/solar.ino
 */

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <ArduinoJson.h>

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

// Pines
#define ADC_PIN 36       // ADC0 (GPIO36 en ESP32)
#define LED_NEGATIVO 32  // energy < 0
#define LED_0_10 33      // 0 <= energy < 10
#define LED_10_20 26     // 10 <= energy <= 20

// LCD I2C (0x27 o 0x3F según tu módulo)
static LiquidCrystal_I2C lcd(0x27, 16, 2);

static unsigned long lastRequestTime = 0;
static const unsigned long requestInterval = 5000;  // 5 segundos

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

  analogReadResolution(12);  // ESP32: 12 bits (0-4095)

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Starting...");

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
  int adcValue = analogRead(ADC_PIN);
  float energy = -10.0f + (adcValue / 4095.0f) * 30.0f;

  controlarLEDs(energy);
  mostrarEnLCD(adcValue, energy);

  unsigned long currentTime = millis();
  if (currentTime - lastRequestTime >= requestInterval) {
    enviarDatosAlServidor(energy);
    lastRequestTime = currentTime;
  }

  delay(100);
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
    String response = http.getString();
    Serial.println("Respuesta: " + response);
  } else {
    Serial.print("Error en POST: ");
    Serial.print(httpResponseCode);
    if (httpResponseCode == -1) {
      Serial.println(" - Error de conexión. Verifica:");
      Serial.println("   1. Que la IP del servidor sea correcta");
      Serial.println("   2. Que el servidor esté corriendo en el puerto 3000");
      Serial.println("   3. Que el ESP32 y el servidor estén en la misma red");
    } else if (httpResponseCode == -2) {
      Serial.println(" - Timeout de conexión");
    } else {
      Serial.println(" - Error desconocido");
    }
  }

  http.end();
}
