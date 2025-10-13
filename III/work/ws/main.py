import asyncio
import json
import logging
from datetime import datetime
from typing import Set
import paho.mqtt.client as mqtt
from aiohttp import web
import aiohttp

logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)


class MQTTWebSocketBridge:
    def __init__(self):
        self.websockets: Set[web.WebSocketResponse] = set()
        self.mqtt_client = mqtt.Client()
        self.setup_mqtt()

    def setup_mqtt(self):
        self.mqtt_client.on_connect = self.on_mqtt_connect
        self.mqtt_client.on_message = self.on_mqtt_message

        # Connect to MQTT broker
        try:
            self.mqtt_client.connect("mosquitto", 1883, 60)
            self.mqtt_client.loop_start()
        except Exception as e:
            logger.error(f"MQTT connection failed: {e}")

    def on_mqtt_connect(self, client, userdata, flags, rc):
        logger.info(f"Connected to MQTT broker with code {rc}")

        client.subscribe("sensors/dht11")
        client.subscribe("sensors/mq2")
        client.subscribe("alerts")

    def on_mqtt_message(self, client, userdata, msg):
        try:
            payload = msg.payload.decode('utf-8')
            data = {
                'topic': msg.topic,
                'timestamp': datetime.now().isoformat(),
                'data': json.loads(payload)
            }
            asyncio.create_task(self.broadcast_to_websockets(data))
        except Exception as e:
            logger.error(f"Error processing MQTT message: {e}")

    async def broadcast_to_websockets(self, data):
        if self.websockets:
            message = json.dumps(data)
            await asyncio.gather(
                *[ws.send_str(message) for ws in self.websockets],
                return_exceptions=True
            )

    async def websocket_handler(self, request):
        ws = web.WebSocketResponse()
        await ws.prepare(request)

        self.websockets.add(ws)
        logger.info(f"WebSocket connected. Total: {len(self.websockets)}")

        try:
            async for msg in ws:
                if msg.type == aiohttp.WSMsgType.TEXT:
                    try:
                        data = json.loads(msg.data)
                        # Handle commands from Android app
                        if 'action' in data:
                            self.handle_command(data)
                    except json.JSONDecodeError:
                        logger.error("Invalid JSON from WebSocket")
                elif msg.type == aiohttp.WSMsgType.ERROR:
                    logger.error(f"WebSocket error: {ws.exception()}")
        finally:
            self.websockets.discard(ws)
            logger.info(
                f"WebSocket disconnected. Total: {len(self.websockets)}")

        return ws

    def handle_command(self, data):
        action = data.get('action')
        logger.info(f"Received command: {action}")
        # Publish command to MQTT
        self.mqtt_client.publish("commands", json.dumps(data))

    async def health_check(self, request):
        return web.json_response({
            'status': 'healthy',
            'websockets': len(self.websockets),
            'mqtt_connected': self.mqtt_client.is_connected()
        })


def create_app():
    bridge = MQTTWebSocketBridge()
    app = web.Application()

    app.router.add_get('/ws', bridge.websocket_handler)
    app.router.add_get('/health', bridge.health_check)

    return app


if __name__ == '__main__':
    app = create_app()
    web.run_app(app, host='0.0.0.0', port=8080)
