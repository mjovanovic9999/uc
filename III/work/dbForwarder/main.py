import json
import time
import paho.mqtt.client as mqtt
from influxdb_client import InfluxDBClient, Point
from influxdb_client.client.write_api import SYNCHRONOUS
import logging

MQTT_BROKER = 'mosquitto'
MQTT_PORT =  1883
INFLUXDB_URL = 'http://influxdb:8181'
INFLUXDB_TOKEN = 'apiv3_maFmbho3ZAMM7w7xrE6dAuLe75C87Kmz0FnD1wEklDHZaDYgVBsQOxxuUKHd0qONiCfG2gx-UdQNFcUZvO22Jw'
INFLUXDB_ORG = 'uc'
INFLUXDB_BUCKET = 'uc'

MQTT_TOPICS = [
    "sensors/mq2",
    "sensors/dht11",
]

class DbForwarder:
    def __init__(self):
        logging.basicConfig(level=logging.INFO)
        self.influx_client = InfluxDBClient(
            url=INFLUXDB_URL,
            token=INFLUXDB_TOKEN,
            org=INFLUXDB_ORG
        )
        self.write_api = self.influx_client.write_api(write_options=SYNCHRONOUS)
        
        self.mqtt_client = mqtt.Client(
            client_id="",
            userdata=None,
            protocol=mqtt.MQTTv311,
            transport="tcp"
        )
        
        self.mqtt_client.on_connect = self.on_connect
        self.mqtt_client.on_message = self.on_message
        
    def on_connect(self, client, userdata, flags, rc):
        logging.info(f"Connected to MQTT broker with result code {rc}")
        for topic in MQTT_TOPICS:
            client.subscribe(topic)
            logging.info(f"Subscribed to topic: {topic}")
    
    def on_message(self, client, userdata, msg):
        try:
            payload = msg.payload.decode('utf-8')
            data = json.loads(payload)
            
            logging.info(f"Received message on {msg.topic}: {data}")
            
            point = Point(self.get_sensor_type(msg.topic))
            
            point.tag("device", "esp32-s3")
            
            for key, value in data.items():
                if isinstance(value, (int, float)):
                    point.field(key, float(value))
            
            point.time(time.time_ns())
            
            self.write_api.write(bucket=INFLUXDB_BUCKET, record=point)
            logging.info(f"Data written to InfluxDB: {point.to_line_protocol()}")
            
        except json.JSONDecodeError as e:
            logging.error(f"Error decoding JSON: {e}")
        except Exception as e:
            logging.error(f"Error processing message: {e}")
    
    def get_sensor_type(self, topic):
        if "mq2" in topic:
            return "mq2"
        elif "dht11" in topic:
            return "dht11"
        else:
            return "unknown"
    
    def start(self):
        try:
            logging.info("Starting MQTT to InfluxDB bridge...")
            self.mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
            self.mqtt_client.loop_forever()
        except KeyboardInterrupt:
            logging.info("Stopping...")
        finally:
            self.mqtt_client.disconnect()
            self.influx_client.close()

if __name__ == "__main__":
    forwarder = DbForwarder()
    forwarder.start()