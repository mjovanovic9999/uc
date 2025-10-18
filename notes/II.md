ssh rpi@192.168.0.17

esp32 192.168.0.18

rsync -avz /home/mj/Desktop/uc rpi@192.168.0.17:/home/rpi/ --exclude 'uc/I/work/.pio'
rsync -avz /home/mj/Desktop/uc/II rpi@10.224.221.14:/home/rpi/Desktop/uc --exclude 'work/esp32/.pio' --exclude 'work/esp32/lib'


10.224.221.14

gpt reko za mosquitto
```c
#include <WiFi.h>
#include <PubSubClient.h>

// ====== WiFi ======
const char* WIFI_SSID     = "YOUR_WIFI_SSID";
const char* WIFI_PASSWORD = "YOUR_WIFI_PASSWORD";

// ====== MQTT (Raspberry Pi broker) ======
const char* MQTT_HOST = "192.168.1.50";  // Pi's LAN IP
const uint16_t MQTT_PORT = 1883;         // Plain MQTT
const char* MQTT_USER = "esp32";
const char* MQTT_PASS = "YOUR_PASSWORD";

// Topics
const char* PUB_TOPIC = "sensors/esp32s3/temperature";
const char* SUB_TOPIC = "sensors/esp32s3/cmd";  // optional control channel

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long lastPublishMs = 0;

void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  Serial.print("MQTT message [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) Serial.print((char)payload[i]);
  Serial.println();
  // Handle commands here
}

void connectWiFi() {
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print('.');
  }
  Serial.printf("\nWiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
}

void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);

  while (!mqtt.connected()) {
    String clientId = "esp32s3-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.printf("Connecting to MQTT as %s ...\n", clientId.c_str());
    if (mqtt.connect(clientId.c_str(), MQTT_USER, MQTT_PASS)) {
      Serial.println("MQTT connected.");
      mqtt.subscribe(SUB_TOPIC);
    } else {
      Serial.printf("MQTT connect failed, rc=%d. Retrying in 2s...\n", mqtt.state());
      delay(2000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  connectWiFi();
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    connectWiFi();
  }
  if (!mqtt.connected()) {
    connectMQTT();
  }
  mqtt.loop();

  // Publish something every 5 seconds
  unsigned long now = millis();
  if (now - lastPublishMs >= 5000) {
    lastPublishMs = now;

    // Fake temperature reading for demo
    float tempC = 25.0 + (float)(millis() % 1000) / 100.0;

    char payload[64];
    snprintf(payload, sizeof(payload), "{\"temp\":%.2f}", tempC);
    bool ok = mqtt.publish(PUB_TOPIC, payload, true);
    Serial.printf("Publish %s: %s\n", ok ? "OK" : "FAILED", payload);
  }
}

```

```bash
sudo chown -R 472:472 ./grafana
sudo chmod -R 750 ./grafana

# Give InfluxDB 3's data dir to UID 1500 (influxdb3 user in the image)
sudo chown -R 1500:1500 ./influxdb/data
sudo chmod -R 750 ./influxdb/data
```

docker compose up -d

mosquitto_pub -m "message from mosquitto_pub client" -t "test"
mosquitto_sub -v -t '#'

mosquitto_pub -m "message from mosquitto_pub client" -t "sensors/esp32s3/cmd"


python:3.14.0-alpine3.22
tensorflow/tensorflow:2.20.0


### a) **Bucket**
- Bucket je **logički kontejner za podatke**, slično kao "baza" ili "shema".    
- U njemu se čuvaju svi podaci iz određene vremenske serije.
- Svaki bucket ima **retention policy** – koliko dugo se podaci čuvaju pre nego što se obrišu.
### b) **Measurement**
- Measurement je **naziv vremenske serije** ili tip podataka koji čuvaš.
- Primer: `temperature`, `cpu_usage`, `stock_price`.
### c) **Tag**
- Tag je **indeksirani atribut** koji pomaže da brzo filtriraš podatke.
- Primer: `location=Belgrade`, `host=server1`.
- Tagovi su stringovi i služe za brzo pretraživanje.
### d) **Field**
- Field je **merna vrednost** ili podatak koji zapravo meriš.
- Primer: `value=23.5` za temperaturu, `usage=75` za CPU.
- Fieldovi **nisu indeksirani**, ali mogu biti različitih tipova (float, int, boolean, string).
### e) **Timestamp**
- Svaka vrednost ima **vreme kada je zabeležena**.
- Timestamp je ključna komponenta TSDB-a jer omogućava analize kroz vreme

sacuvati sve sto treba od RPI config i sl


moze grafana dodatno da se sredi; sub na mqtt alerts npr