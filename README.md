# Weather_station_system
This is a weather station that receives real-time metrics such as temperature and humidity through sensors. Metrics are read and transmitted through ESP32 microcontroller over MQTT and LoRa.

# Prerequisites

- `PubSubClient by Nick O'Leary v2.8`
- `DHT sensor library by Adafruit`
- `Adafruit Unified Sensor`
- `ESP32 board support (Heltec ESP32 Dev-Boards)`
- `WiFi.h : Wi-Fi connectivity.`

# Step 1. Mosquitto MQTT broker in Docker with authentication

1. **Create Username and password**

`docker run -it --rm -v "$PWD/config:/mosquitto/config" eclipse-mosquitto \
  mosquitto_passwd -c /mosquitto/config/pwfile <username>
`

2. **Docker compose setup**

After creating the docker-compose.yml file, we start Mosquitto from inside the `mqtt` directory as:

`docker compose up -d`

# Step 2. ESP32 Code (Arduino IDE) 

 We programmed the ESP32 dev board with C++, accounting for it connecting to WiFi (same as the MQTT broker). The ESP32 reads and processes data from the DHT11 sensor and publishes a topic through connecting to the MQTT broker based on the host IP over open port 1883.

# Step 3. Test the Authentication with Mosquitto Client

Next step involves a subscriber receiving the distributed the topic using the Mosquitto client. Since we were running the Mosquitto on Docker, we run the subscriber using the below command:

`docker run --rm -it eclipse-mosquitto mosquitto_sub -h <host IP> -t weather/data -u <mqttuser> -P <mqttpassword>`

The ESP32 uses these same credentials.

# Step 4. Track real-time data

For this step, we implemented a matplotlib graph that tracks real-time updates of the temperature and humidity readings. It is important to indicate the IP address of the host before executing the program. Using keyboard interrupt will stop the script and display a final summary plot of the readings for the period that the sensor was running in minutes.

`python liveplot.py`