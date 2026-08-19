import paho.mqtt.client as mqtt
import psycopg2

# ==========================
# PostgreSQL Configuration
# ==========================

conn = psycopg2.connect(
    host="localhost",
    database="FINAL_TEST",
    user="postgres",          # Change if your username is different
    password="GAS_SENSOR_123",
    port="5432"
)

cursor = conn.cursor()

print("Connected to PostgreSQL")

# ==========================
# MQTT Configuration
# ==========================

BROKER = "127.0.0.1"
PORT = 1883
TOPIC = "esp32/gas"

# ==========================
# MQTT Callbacks
# ==========================

def on_connect(client, userdata, flags, reason_code, properties=None):
    print("Connected to Mosquitto Broker!")
    client.subscribe(TOPIC)
    print(f"Subscribed to topic: {TOPIC}")

def on_message(client, userdata, msg):
    try:
        gas = int(msg.payload.decode())

        if gas < 700:
            status = "SAFE"
        elif gas < 1350:
            status = "WARNING"
        else:
            status = "DANGER"

        print("--------------------------------")
        print(f"Gas PPM : {gas}")
        print(f"Status  : {status}")

        cursor.execute(
            """
            INSERT INTO gas_sensor_data (gas_ppm, status)
            VALUES (%s, %s)
            """,
            (gas, status)
        )

        conn.commit()

        print("Inserted into PostgreSQL\n")

    except Exception as e:
        print("Database Error:", e)

# ==========================
# Main
# ==========================

client = mqtt.Client(mqtt.CallbackAPIVersion.VERSION2)

client.on_connect = on_connect
client.on_message = on_message

print("Connecting to MQTT Broker...")

client.connect(BROKER, PORT, 60)

print("Waiting for ESP32 data...\n")

client.loop_forever()