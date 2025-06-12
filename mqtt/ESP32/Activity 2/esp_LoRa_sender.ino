#include <SPI.h>
#include <LoRa.h>
#include "DHT.h"
#include <ArduinoJson.h>

/* ---------- Configuration ---------- */
#define DHTPIN     17     /* GPIO connected to DHT11 */
#define DHTTYPE    DHT11
#define DEVICE_ID  "esp32-dht-sender"  /* Unique identifier for sender */

/* LoRa Pin definitions */
#define LORA_SCK   5
#define LORA_MISO  19
#define LORA_MOSI  27
#define LORA_SS    18
#define LORA_RST   14
#define LORA_DI0   26

#define LORA_BAND  860E6   /* Frequency */

/* ---------- Global Objects ---------- */
DHT dht(DHTPIN, DHTTYPE);
unsigned int packetCounter = 0;

/* ---------- Setup Function ---------- */
void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("🚀 Starting LoRa DHT11 Sender");

  /* Initialize DHT11 sensor */
  dht.begin();
  Serial.println("✅ DHT11 initialized");

  /* Initialize LoRa */
  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  LoRa.setSpreadingFactor(7);     // Range: 6–12
  LoRa.setSignalBandwidth(125E3); // e.g., 62.5E3, 125E3, 250E3
  // LoRa.setCodingRate4(5);         // Valid: 5–8 (CR 4/x)
  LoRa.setTxPower(10); // Range: 2–20 dBm
  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("❌ Starting LoRa failed!");
    while (true);
  }
  Serial.println("✅ LoRa initialized");
  LoRa.onTxDone(onTxDone);  /* Optional callback */
}

/* ---------- Main Loop ---------- */
void loop() {
    delay(3000);  
    float h = dht.readHumidity();
    float tC = dht.readTemperature();     /* Celsius */
    float tF = dht.readTemperature(true); /* Fahrenheit */

    if (isnan(h) || isnan(tC) || isnan(tF)) {
      Serial.println("{\"error\":\"Failed to read from DHT sensor\"}");
      return;
    }

    float hic = dht.computeHeatIndex(tC, h, false);
    float hif = dht.computeHeatIndex(tF, h);

    /* Create JSON document */
    StaticJsonDocument<256> doc;
    doc["id"] = DEVICE_ID;
    doc["humidity"] = h;
    doc["temperature_c"] = tC;
    doc["temperature_f"] = tF;
    doc["heat_index_c"] = hic;
    doc["heat_index_f"] = hif;
    doc["packet"] = packetCounter;

    /* Serialize to string */
    String jsonString;
    serializeJsonPretty(doc, jsonString);

    /* Debug output */
    Serial.println("📦 Sending JSON payload over LoRa:");
    Serial.println(jsonString);

    /* Send over LoRa (non-blocking) */
    LoRa.beginPacket();
    LoRa.print(jsonString);
    LoRa.endPacket(true);  // true = async / non-blocking

    packetCounter++;
}

/* ---------- Callback on Transmission Done ---------- */
void onTxDone() {
  Serial.println("✅ Transmission completed (TxDone)");
}
