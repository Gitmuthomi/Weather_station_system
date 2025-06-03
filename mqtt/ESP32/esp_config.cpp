#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

const char* ssid = "wifi_user";
const char* password = "wifi_password";
const char* mqtt_server = "192.168.x.x";  // <-- IP of your computer
const int mqtt_port = 1883;
const char* mqtt_user = "ricopol";
const char* mqtt_pass = "ricopol";

WiFiClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);

void setup_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP32Client", mqtt_user, mqtt_pass)) {
      // Success
    } else {
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
}

void loop() {
  if (!client.connected()) reconnect();
  client.loop();

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  if (!isnan(h) && !isnan(t)) {
    String payload = "{\"temperature\":" + String(t) + ",\"humidity\":" + String(h) + "}";
    client.publish("weather/data", payload.c_str());
  }

  delay(5000);
}

