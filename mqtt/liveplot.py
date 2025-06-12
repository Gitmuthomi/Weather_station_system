import matplotlib.pyplot as plt
import matplotlib.animation as animation
import json
import time
from paho.mqtt import client as mqtt_client
from collections import deque

# MQTT configuration
broker = '192.168.1.131'
port = 1883
topic = "weather/data"
username = 'ricopol'
password = 'ricopol'

# Data buffers
max_len = 300  # Show last 5 minutes at 1Hz
timestamps = deque(maxlen=max_len)
temps = deque(maxlen=max_len)
hums = deque(maxlen=max_len)
start_time = time.time()

# MQTT callbacks
def on_connect(client, userdata, flags, rc):
    client.subscribe(topic)

def on_message(client, userdata, msg):
    try:
        data = json.loads(msg.payload.decode())
        now = time.time() - start_time
        temps.append(data["temperature_c"])
        hums.append(data["humidity"])
        timestamps.append(now)
    except Exception as e:
        print("Parse error:", e)

# MQTT setup
client = mqtt_client.Client()
client.username_pw_set(username, password)
client.on_connect = on_connect
client.on_message = on_message
client.connect(broker, port)
client.loop_start()

# Matplotlib setup
fig, ax1 = plt.subplots()
ax2 = ax1.twinx()

temp_line, = ax1.plot([], [], 'r-', label="Temp (°C)")
hum_line, = ax2.plot([], [], 'b-', label="Humidity (%)")

ax1.set_ylabel('Temperature (°C)', color='r')
ax2.set_ylabel('Humidity (%)', color='b')
ax1.set_xlabel('Time (min)')
ax1.set_ylim(10, 40)
ax2.set_ylim(0, 100)

def update(frame):
    if timestamps:
        # Convert seconds to minutes for x-axis
        x_vals = [t / 60 for t in timestamps]
        temp_line.set_data(x_vals, temps)
        hum_line.set_data(x_vals, hums)
        ax1.set_xlim(x_vals[0], x_vals[-1])
    return temp_line, hum_line

ani = animation.FuncAnimation(fig, update, interval=1000)

try:
    plt.title("Real-time Weather Monitoring")
    plt.show()
except KeyboardInterrupt:
    pass
finally:
    client.loop_stop()
    client.disconnect()

    # Final static plot
    plt.figure()
    ax1 = plt.gca()
    ax2 = ax1.twinx()
    x_vals = [t / 60 for t in timestamps]
    ax1.plot(x_vals, temps, 'r-', label="Temp (°C)")
    ax2.plot(x_vals, hums, 'b-', label="Humidity (%)")
    ax1.set_xlabel("Time (min)")
    ax1.set_ylabel("Temperature (°C)", color='r')
    ax2.set_ylabel("Humidity (%)", color='b')
    plt.title("Sensor Data Summary")
    plt.tight_layout()
    plt.show()
