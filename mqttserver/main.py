import paho.mqtt.client as mqtt
import json
from prometheus_client import start_http_server, Gauge

# Prometheus metrics - Sử dụng một dictionary để lưu trữ metrics cho mỗi node
metrics = {}

# MQTT Configuration
MQTT_BROKER = "mqtt.myvpn.id.vn"
MQTT_PORT = 1883

def get_node_metrics(node_id):
    """Trả về metrics cho một node cụ thể, tạo mới nếu chưa tồn tại."""
    if node_id not in metrics:
        metrics[node_id] = {
            'soil_temperature': Gauge(f'soil_temperature_{node_id}', f'Soil temperature in Celsius for node {node_id}'),
            'soil_ph': Gauge(f'soil_ph_{node_id}', f'Soil pH value for node {node_id}'),
            'bmp280_temperature': Gauge(f'bmp280_temperature_{node_id}', f'BMP280 temperature in Celsius for node {node_id}'),
            'bmp280_pressure': Gauge(f'bmp280_pressure_{node_id}', f'BMP280 pressure in Pa for node {node_id}'),
        }
    return metrics[node_id]

def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    client.subscribe("to-server/#")

def on_message(client, userdata, msg):
    try:
        topic_parts = msg.topic.split("/")
        if len(topic_parts) >= 2 and topic_parts[0] == "to-server":
            node_id = topic_parts[1]
            node_metrics = get_node_metrics(node_id) # Lấy hoặc tạo metrics cho node

            payload = msg.payload.decode()
            data = json.loads(payload)

            if "data" in data and "message_type" in data["data"]:
                message_type = data["data"]["message_type"]
                if message_type == "measurement":
                    print(f"Message received from topic: {msg.topic}")
                    measurements = data["data"].get("message", [])
                    for measurement in measurements:
                        measurement_value = measurement.get("measurement")
                        measurement_type = measurement.get("type")
                        if measurement_value is not None and measurement_type is not None:
                            print(f"  Type: {measurement_type}, Measurement: {measurement_value}")
                            # Cập nhật metric tương ứng cho node
                            if measurement_type == "Soil_Temperature":
                                node_metrics['soil_temperature'].set(measurement_value)
                            elif measurement_type == "Soil_PH":
                                node_metrics['soil_ph'].set(measurement_value)
                            elif measurement_type == "BMP280_Temperature":
                                node_metrics['bmp280_temperature'].set(measurement_value)
                            elif measurement_type == "BMP280_Pressure":
                                node_metrics['bmp280_pressure'].set(measurement_value)
                            else:
                                print(f"Unknown measurement type: {measurement_type}")
                        else:
                            print("Invalid measurement format")
                else:
                    print(f"Message type is not 'measurement': {message_type}")
            else:
                print("Invalid message format: missing 'data' or 'message_type'")
        else:
            print(f"Invalid topic format: {msg.topic}")


    except json.JSONDecodeError:
        print(f"Invalid JSON received from topic: {msg.topic}: {payload}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

# Start Prometheus HTTP server
start_http_server(8000)

client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
client.connect(MQTT_BROKER, MQTT_PORT, 60)

client.loop_forever()