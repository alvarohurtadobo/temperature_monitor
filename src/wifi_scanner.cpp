/**
 * WiFi Scanner - Detecta si Maxwell y Curie son 2.4 GHz o 5 GHz.
 * Canales 1-13 = 2.4 GHz (ESP32 compatible)
 * Canales 36+ = 5 GHz (ESP32 NO compatible)
 */

#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n\n=== WiFi Scanner (Maxwell / Curie) ===\n");
}

void loop() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);

  Serial.println("Escaneando...");
  int n = WiFi.scanNetworks();

  Serial.printf("Encontradas %d redes\n\n", n);

  for (int i = 0; i < n; i++) {
    String ssid = WiFi.SSID(i);
    // Solo mostrar Maxwell y Curie (o todas si quieres: quita el if)
    if (ssid.equalsIgnoreCase("Maxwell") || ssid.equalsIgnoreCase("Curie")) {
      int ch = WiFi.channel(i);
      const char* band = (ch >= 1 && ch <= 13) ? "2.4 GHz" : "5 GHz";
      Serial.printf("  %s  ->  Canal %2d  ->  %s  (RSSI: %d dBm)\n",
                    ssid.c_str(), ch, band, WiFi.RSSI(i));
    }
  }

  // Mostrar todas las redes (opcional, para debug)
  Serial.println("\n--- Todas las redes ---");
  for (int i = 0; i < n; i++) {
    int ch = WiFi.channel(i);
    const char* band = (ch >= 1 && ch <= 13) ? "2.4" : "5G";
    Serial.printf("  %-25s  Ch %2d  %s  %d dBm\n",
                  WiFi.SSID(i).c_str(), ch, band, WiFi.RSSI(i));
  }

  Serial.println("\n--- Repetir en 10 s ---\n");
  delay(10000);
}
