import asyncio
import json
import logging
from datetime import datetime
from typing import Set
import paho.mqtt.client as mqtt
from aiohttp import web
import aiohttp

MQTT_BROKER = 'mosquitto'
MQTT_PORT = 1883

logging.basicConfig(level=logging.INFO)


class MQTTWebSocketBridge:
    def __init__(self, loop):
        self.websockets: Set[web.WebSocketResponse] = set()
        self.mqtt_client = mqtt.Client(
            client_id="",
            userdata=None,
            protocol=mqtt.MQTTv311,
            transport="tcp"
        )
        self.loop = loop
        self.setup_mqtt()

    def setup_mqtt(self):
        self.mqtt_client.on_connect = self.on_mqtt_connect
        self.mqtt_client.on_message = self.on_mqtt_message

        # Connect to MQTT broker
        try:
            # self.mqtt_client.connect(MQTT_BROKER, MQTT_PORT, 60)
            self.mqtt_client.connect_async(MQTT_BROKER, MQTT_PORT, 60)
            self.mqtt_client.loop_start()
        except Exception as e:
            logging.error(f"MQTT connection failed: {e}")

    def on_mqtt_connect(self, client, userdata, flags, rc):
        logging.info(f"Connected to MQTT broker with code {rc}")

        client.subscribe("sensors/mq2")
        client.subscribe("sensors/dht11")
        client.subscribe("alerts")

    def on_mqtt_message(self, client, userdata, msg):
        logging.info(
            f"MQTT message received - Topic: {msg.topic}, Payload: {msg.payload}")
        try:
            payload = msg.payload.decode('utf-8')
            data=json.loads(payload)
            logging.info(
            f"MQTT {data}")
            asyncio.run_coroutine_threadsafe(
                self.broadcast_to_websockets(data),
                self.loop
            )

        except Exception as e:
            logging.error(f"Error processing MQTT message: {e}")

    async def broadcast_to_websockets(self, data):
        if not self.websockets:
            logging.info("No websockets to broadcast to.")
            return

        message = json.dumps(data)
        logging.info(f"Broadcasting {message}")
        await asyncio.gather(
            *[ws.send_str(message) for ws in self.websockets],
            return_exceptions=True
        )

    async def websocket_handler(self, request):
        ws = web.WebSocketResponse()
        await ws.prepare(request)

        self.websockets.add(ws)
        logging.info(f"WebSocket connected. Total: {len(self.websockets)}")

        try:
            async for msg in ws:
                if msg.type == aiohttp.WSMsgType.TEXT:
                    try:
                        data = json.loads(msg.data)
                        if 'action' in data:
                            self.handle_command(data)
                    except json.JSONDecodeError:
                        logging.error("Invalid JSON from WebSocket")
                elif msg.type == aiohttp.WSMsgType.ERROR:
                    logging.error(f"WebSocket error: {ws.exception()}")
        finally:
            self.websockets.discard(ws)
            logging.info(
                f"WebSocket disconnected. Total: {len(self.websockets)}")

        return ws

    def handle_command(self, data):
        action = data.get('action')
        logging.info(f"Received command: {action}")

        self.mqtt_client.publish("commands", json.dumps(data))

    async def health_check(self, request):
        return web.json_response({
            'status': 'healthy',
            'websockets': len(self.websockets),
            'mqtt_connected': self.mqtt_client.is_connected()
        })


def create_app(loop):
    bridge = MQTTWebSocketBridge(loop)
    app = web.Application()

    app.router.add_get('/', bridge.websocket_handler)
    app.router.add_get('/health', bridge.health_check)

    return app

if __name__ == '__main__':
    async def create_app_and_run():
        loop = asyncio.get_running_loop()
        app = create_app(loop)
        return app

    web.run_app(create_app_and_run(), host='0.0.0.0', port=8080)
