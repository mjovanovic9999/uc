import paho.mqtt.client as mqtt
import tflite_runtime.interpreter as tflite
import numpy as np
import pickle
import json
import logging

MQTT_BROKER = "mosquitto"
MQTT_PORT = 1883

interpreter = tflite.Interpreter(model_path="sensor_model.tflite")
interpreter.allocate_tensors()
input_details = interpreter.get_input_details()
output_details = interpreter.get_output_details()

with open('scaler.pkl', 'rb') as f:
    scaler = pickle.load(f)

LABELS = ['normal', 'gas_leak', 'fire', 'high_humidity']

sensor_data = {'mq2': 0, 'temperature': 22, 'humidity': 45}

current_label = "normal"
previous_label = "normal"

logging.basicConfig(level=logging.INFO)


def predict():
    input_data = np.array([[
        sensor_data['mq2'],
        sensor_data['temperature'],
        sensor_data['humidity']
    ]], dtype=np.float32)

    input_data = scaler.transform(input_data)

    interpreter.set_tensor(input_details[0]['index'], input_data)
    interpreter.invoke()
    output = interpreter.get_tensor(output_details[0]['index'])[0]

    pred_class = np.argmax(output)
    confidence = output[pred_class]

    return LABELS[pred_class], confidence


def on_connect(client, userdata, flags, rc):
    logging.info(f"Connected to MQTT broker (rc={rc})")
    client.subscribe("sensors/mq2")
    client.subscribe("sensors/dht11")


def on_message(client, userdata, msg):
    global current_label, previous_label
    try:
        data = json.loads(msg.payload.decode())
        logging.info(f"Received json{data})")
        
        if msg.topic == "sensors/mq2":
            sensor_data['mq2'] = data.get('quality', 0)
        elif msg.topic == "sensors/dht11":
            sensor_data['temperature'] = data.get('temperature', 0)
            sensor_data['humidity'] = data.get('humidity', 0)

        previous_label = current_label
        current_label, confidence = predict()

        logging.info(f"MQ2={sensor_data['mq2']:.2f}% T={sensor_data['temperature']:.1f}°C "
                     f"H={sensor_data['humidity']:.1f}% → {current_label} ({confidence:.1%})")

        if current_label != previous_label:
            alert = {
                'type': current_label,
                'confidence': float(confidence),
                'mq2': sensor_data['mq2'],
                'temperature': sensor_data['temperature'],
                'humidity': sensor_data['humidity']
            }
            client.publish("alerts", json.dumps(alert))
            logging.info(f"alert is: {current_label}")

    except Exception as e:
        logging.error(f"Error: {e}")


client = mqtt.Client(
    client_id="",
    userdata=None,
    protocol=mqtt.MQTTv311,
    transport="tcp"
)
client.on_connect = on_connect
client.on_message = on_message

logging.info("Starting ML inference service...")
client.connect(MQTT_BROKER, MQTT_PORT, 60)
client.loop_forever()
