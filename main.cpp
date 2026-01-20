#include <Arduino.h>
#include <ArduinoJson.h>
#include <LiquidCrystal_I2C.h>
#include <ModbusRtu.h>
#include <NimBLEDevice.h>
#include <WebSocketsServer.h>
#include <WiFi.h>
#include <Wire.h>
#include <max6675.h>

// Identificadores BLE
#define DEVICE_NAME "ESP32_DELTA_01"
#define SERVICE_UUID "12345678-1234-1234-1234-1234567890ab"
#define CHAR_UUID_NOTIFY "87654321-4321-4321-4321-ba0987654321"

// Pines termocuplas (MAX6675)
static const int thermoCSTT = 23;  // TT probe CS
static const int thermoCSTA = 18;  // TA probe CS
static const int thermoCSTG = 17;  // TG probe CS
static const int thermoCLK = 5;    // SCK/CLK
static const int thermoDO = 19;    // SO/DO

// Lecturas en tiempo real
static float TKTG;  // Bean temperature
static float TKTA;  // Exhaust air temperature
static float TKTT;  // Optional third probe
static float E;     // TT percentage (legacy)
static float DT;    // Delta between TA and TG

// WiFi (credenciales de ejemplo; mover a secrets si aplica)
static const char* ssid = "Maxwell";
static const char* password = "299792458ms";

// WebSocket server (Artisan)
static WebSocketsServer webSocket(81);

// BLE
static NimBLEServer* pServer = nullptr;
static NimBLECharacteristic* pCharacteristic = nullptr;

class MyServerCallbacks : public NimBLEServerCallbacks {
  void onDisconnect(NimBLEServer* server, NimBLEConnInfo& connInfo, int reason) override {
    (void)connInfo;
    (void)reason;
    Serial.println("Cliente desconectado, reiniciando advertising...");
    server->startAdvertising();
  }
};

// Modbus data array (valores compartidos)
static uint16_t au16data[16] = {
  120, 145, 177, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0xFFFF
};

// Modbus slave @ ID 1 usando Serial
static Modbus slave(1, Serial, 0);

// Lectores MAX6675
static MAX6675 thermocoupleTG(thermoCLK, thermoCSTG, thermoDO);
// static MAX6675 thermocoupleTA(thermoCLK, thermoCSTA, thermoDO);
// static MAX6675 thermocoupleTT(thermoCLK, thermoCSTT, thermoDO);

// LCD (20x4)
static LiquidCrystal_I2C lcd(0x27, 20, 4);

static int counter = 0;

static void handleArtisanMessage(uint8_t num, const String& payload) {
  Serial.println("Recibido de Artisan: " + payload);

  StaticJsonDocument<256> doc;
  DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.println("Error parseando JSON");
    return;
  }

  const char* command = doc["command"];
  if (command && String(command) == "getData") {
    int id = doc["id"] | 0;

    StaticJsonDocument<256> resp;
    resp["id"] = id;

    JsonObject data = resp.createNestedObject("data");
    data["BT"] = TKTG;
    data["ET"] = TKTA;
    data["T1"] = TKTT;

    String out;
    serializeJson(resp, out);
    webSocket.sendTXT(num, out);
    Serial.println("Enviado a Artisan: " + out);
  }
}

static void onWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  (void)length;
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("Artisan conectado al socket #%u\n", num);
      break;
    case WStype_DISCONNECTED:
      Serial.printf("Artisan desconectado del socket #%u\n", num);
      break;
    case WStype_TEXT:
      handleArtisanMessage(num, String(reinterpret_cast<char*>(payload)));
      break;
    default:
      break;
  }
}

void setup() {
  // LCD
  lcd.init();
  lcd.backlight();

  // Modbus
  slave.begin(19200);
  slave.start();

  // BLE (notificaciones a clientes)
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

  // WiFi + WebSocket (Artisan)
  WiFi.begin(ssid, password);
  Serial.println("\nConectando a WiFi...");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConectado a WiFi");
  Serial.print("IP del ESP32: ");
  Serial.println(WiFi.localIP());

  webSocket.begin();
  webSocket.onEvent(onWebSocketEvent);
  Serial.println("Servidor WebSocket escuchando en puerto 81");
}

void loop() {
  webSocket.loop();
  counter++;

  // Lecturas reales (habilitar cuando estén conectadas las sondas)
  // TKTG = thermocoupleTG.readCelsius();
  // TKTA = thermocoupleTA.readCelsius();
  // TKTT = thermocoupleTT.readCelsius();

  E = TKTT * 100 / 150;
  DT = TKTA - TKTG;

  au16data[0] = static_cast<uint16_t>(TKTG);
  au16data[1] = static_cast<uint16_t>(TKTA);
  au16data[2] = static_cast<uint16_t>(TKTT);

  // Datos simulados (legacy) para pruebas sin sensores
  TKTG = 100 + millis() / 10000;
  TKTA = 156 + millis() / 50000;
  TKTT = 170 + millis() / 2000;

  // Layout LCD
  lcd.setCursor(2, 0);
  lcd.print("DELTA TOSTADORAS ");
  lcd.setCursor(15, 1);
  lcd.print("oct24");
  lcd.setCursor(1, 2);
  lcd.print("TG");
  lcd.setCursor(1, 3);
  lcd.print(TKTG, 0);
  lcd.print("C");
  lcd.setCursor(7, 2);
  lcd.print("TA");
  lcd.setCursor(7, 3);
  lcd.print(TKTA, 0);
  lcd.print("C");
  lcd.setCursor(12, 2);
  lcd.print("DT");
  lcd.setCursor(12, 3);
  lcd.print(DT, 0);
  lcd.print("C");
  lcd.setCursor(17, 2);
  lcd.print("E");
  lcd.setCursor(17, 3);
  lcd.print(E, 0);
  lcd.print("%");

  slave.poll(au16data, 16);

  String serializedData = "[";
  for (int i = 0; i < 16; i++) {
    serializedData += String(au16data[i]);
    if (i < 15) {
      serializedData += ",";
    }
  }
  serializedData += "]";

  // Notifica por BLE cada ~1.2 s (150 ms * 8)
  if (counter % 8 == 0) {
    pCharacteristic->setValue(
      reinterpret_cast<uint8_t*>(const_cast<char*>(serializedData.c_str())),
      serializedData.length());
    pCharacteristic->notify();
  }

  // Log por Serial cada ~4.5 s (150 ms * 30)
  if (counter % 30 == 0) {
    Serial.println();
    Serial.println("Last notified: " + serializedData);
    Serial.println();
  }

  delay(150);
}
