#include <SPI.h>
#include <LoRa.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

/* ---------- LoRa Pins and Frequency ---------- */
#define LORA_SCK   5
#define LORA_MISO  19
#define LORA_MOSI  27
#define LORA_SS    18
#define LORA_RST   14
#define LORA_DI0   26
#define LORA_BAND  860E6

/* ---------- WiFi & MQTT Configuration ---------- */
const char* ssid = "Eric's iPhone";
const char* password = "Uzitombaya";

const char* mqtt_server = "172.20.10.7";
const int mqtt_port = 1883;
const char* mqtt_user = "ricopol";
const char* mqtt_pass = "ricopol";
const char* topic = "weather/data";

WiFiClient espClient;
PubSubClient mqttClient(espClient);

/* ---------- Functions ---------- */

void setup_wifi() {
  Serial.print("Connecting to WiFi ");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500); Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: " + WiFi.localIP().toString());
}

void mqtt_reconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect("LoRaBridge", mqtt_user, mqtt_pass)) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial);

  Serial.println("Starting LoRa → MQTT Bridge");

  setup_wifi();

  mqttClient.setServer(mqtt_server, mqtt_port);

  LoRa.setPins(LORA_SS, LORA_RST, LORA_DI0);
  LoRa.setSpreadingFactor(7);     // Range: 6–12. Controls range and data rate
  LoRa.setSignalBandwidth(125E3); // (62.5E3, 125E3, 250E3)
  // LoRa.setCodingRate4(5);         // x = 5–8 (CR 4/x); adds redundancy for error correction

  if (!LoRa.begin(LORA_BAND)) {
    Serial.println("LoRa init failed");
    while (true);
  }
  Serial.println("LoRa Initialized");
}

void loop() {
  if (!mqttClient.connected()) {
    mqtt_reconnect();
  }
  mqttClient.loop();

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    String received = "";
    while (LoRa.available()) {
      received += (char)LoRa.read();
    }

    Serial.println("LoRa Received:");
    Serial.println(received);

    // Parse JSON and re-serialize to ensure it's valid before publishing
    StaticJsonDocument<256> doc;
    DeserializationError err = deserializeJson(doc, received);
    if (err) {
      Serial.print("JSON error: ");
      Serial.println(err.c_str());
      return;
    }
    // Packet loss tracking
    static int lastPacket = -1;
    int currentPacket = doc["packet"];

    if (lastPacket != -1 && currentPacket != lastPacket + 1) {
    Serial.printf("❌ Packet(s) lost: expected %d but got %d\n", lastPacket + 1, currentPacket);
    }
    lastPacket = currentPacket;

    // Get RSSI(Signal Strength) and SNR(Signal-to-Noise ratio)
    int rssi = LoRa.packetRssi();
    float snr = LoRa.packetSnr();
    Serial.printf("Packet %d | RSSI: %d dBm | SNR: %.2f dB\n", currentPacket, rssi, snr);
    // Convert JSON doc back to string
    String payload;
    serializeJson(doc, payload);

    // Publish to MQTT
    mqttClient.publish(topic, payload.c_str());
    Serial.println("Published to MQTT:");
    Serial.println(payload);
  }
}
