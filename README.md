# Weather_station_system
This is a weather station that receives real-time metrics such as temperature and humidity through sensors. Metrics are read and transmitted through ESP32 microcontroller over MQTT and LoRa.

# Prerequisites

- `PubSubClient by Nick O'Leary v2.8`
- `DHT sensor library by Adafruit`
- `Adafruit Unified Sensor`
- `ESP32 board support (Heltec ESP32 Dev-Boards)`

# Step 1. Mosquitto MQTT broker in Docker with authentication

1. **Create Username and password**

`docker run -it --rm -v "$PWD/config:/mosquitto/config" eclipse-mosquitto \
  mosquitto_passwd -c /mosquitto/config/pwfile <user>
`

2. **Docker compose setup**

After creating the docker-compose.yaml file, we start Mosquitto from inside the `mqtt` directory as:

`docker compose up -d`

# Step 2. ESP32 Code (Arduino IDE) 


# Step 3. Test the Authentication with Mosquitto Client

On the host (PC), we run:

`mosquitto_sub -h localhost -t weather/data -u user -P password`

If successful, the ESP32 can use these same credentials.