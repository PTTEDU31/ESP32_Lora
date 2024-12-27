import paho.mqtt.client as mqtt
from influxdb_client_3 import InfluxDBClient3, Point
from datetime import datetime
import json

# MQTT Broker Configuration
MQTT_BROKER = "mqtt.myvpn.id.vn"
MQTT_PORT = 1883
topic = "to-server/+"  # Subscribe to all topics under "to-server/"

# InfluxDB Configuration
INFLUXDB_TOKEN='XZcFpqtitMtlzChkD5ZkkjFvKztx6RSO-btV3YBN4pLGIIb-mdlCD81kOaeZafSRMVVODFHxu1Z67GIlFwILoQ=='
org = "de team"
bucket = "de_team_bucket"
host = "https://us-east-1-1.aws.cloud2.influxdata.com"

# Initialize InfluxDB client with database parameter
influxdb_client = InfluxDBClient3(host=host, token=INFLUXDB_TOKEN, org=org, database=bucket)

# Prometheus Metrics (using a dictionary)
metrics = {}

def create_gauge(metric_name):
    """Creates a gauge metric for the given name."""
    from prometheus_client import Gauge
    return Gauge(metric_name, 'Sensor measurement')

def send_to_influxdb(data, node_id):
    """
    Sends measurement data to InfluxDB.
    
    Args:
        data (dict): The parsed JSON data containing measurements
        node_id (str): The ID of the node sending the data
    """
    try:
        measurements = data["data"].get("message", [])
        timestamp = datetime.utcnow()

        # Process each measurement
        for measurement in measurements:
            measurement_value = measurement.get("measurement")
            measurement_type = measurement.get("type")
            
            if measurement_value is not None and measurement_type is not None:
                # Create a Point object
                point = Point(measurement_type) \
                    .field("value", float(measurement_value)) \
                    .tag("node_id", node_id) \
                    .tag("app_port_src", data["data"].get("appPortSrc")) \
                    .tag("app_port_dst", data["data"].get("appPortDst")) \
                    .time(timestamp)

                # Write to InfluxDB - using just point without bucket parameter
                influxdb_client.write(point)
                print(f"Sent to InfluxDB - Type: {measurement_type}, Value: {measurement_value}")

    except Exception as e:
        print(f"Error sending data to InfluxDB: {e}")

def on_connect(client, userdata, flags, rc):
    """Handles successful connection to the MQTT broker."""
    if rc == 0:
        print("Connected to MQTT broker!")
        client.subscribe(topic)
    else:
        print(f"Connection failed with code {rc}")

def on_message(client, userdata, msg):
    """Processes incoming MQTT messages."""
    try:
        # Extract node ID from the topic
        topic_parts = msg.topic.split("/")
        if len(topic_parts) >= 2 and topic_parts[0] == "to-server":
            node_id = topic_parts[1]
            
            # Decode and parse JSON payload
            payload = msg.payload.decode()
            data = json.loads(payload)
            
            if "data" in data and "message_type" in data["data"]:
                message_type = data["data"]["message_type"]
                
                if message_type == "measurement":
                    print(f"Message received from topic: {msg.topic}")
                    measurements = data["data"].get("message", [])
                    
                    # Update Prometheus metrics
                    for measurement in measurements:
                        measurement_value = measurement.get("measurement")
                        measurement_type = measurement.get("type")
                        
                        if measurement_value is not None and measurement_type is not None:
                            if measurement_type not in metrics:
                                metrics[measurement_type] = create_gauge(measurement_type)
                            metrics[measurement_type].set(measurement_value)
                            print(f"  Type: {measurement_type}, Measurement: {measurement_value}")
                    
                    # Send data to InfluxDB
                    send_to_influxdb(data, node_id)
                else:
                    print(f"Message type is not 'measurement': {message_type}")
            else:
                print(f"Invalid message format: missing 'data' or 'message_type'")
        else:
            print(f"Invalid topic format: {msg.topic}")
            
    except json.JSONDecodeError:
        print(f"Invalid JSON received from topic: {msg.topic}")
    except Exception as e:
        print(f"An unexpected error occurred: {e}")

def main():
    # Create MQTT client
    mqtt_client = mqtt.Client()
    mqtt_client.on_connect = on_connect
    mqtt_client.on_message = on_message

    try:
        # Connect to MQTT broker
        mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
        mqtt_client.loop_forever()
    except KeyboardInterrupt:
        print("Shutting down...")
        mqtt_client.disconnect()
        influxdb_client.close()
    except Exception as e:
        print(f"Error in main loop: {e}")

if __name__ == "__main__":
    main()