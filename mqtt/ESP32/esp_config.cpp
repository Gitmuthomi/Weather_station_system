#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 17
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

#define LED_PIN 25

// WiFi credentials
const char* ssid = "wifi username";
const char* password = "wifi pass";

// MQTT broker (Docker host IP)
const char* mqtt_server = "YOUR PC IP";  // 
const int mqtt_port = 1883;
const char* mqtt_user = "ricopol";
const char* mqtt_pass = "ricopol";
const char* topic = "weather/data";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {
  delay(10);
  Serial.println("Connecting to WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected. IP: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Connecting to MQTT...");
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor");
    return;
  }

  // Format payload
  String payload = "{\"temperature\": " + String(t) + ", \"humidity\": " + String(h) + "}";
  Serial.println("Publishing: " + payload);

  // Publish to MQTT
  client.publish(topic, payload.c_str());

  // Blink LED
  digitalWrite(LED_PIN, HIGH);
  delay(200);
  digitalWrite(LED_PIN, LOW);

  delay(5000);
}
