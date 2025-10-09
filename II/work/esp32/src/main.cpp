// #include <Arduino.h>
// #include "DHT.h"

// #define DHTPIN 4  
// #define DHTTYPE DHT11

// DHT dht(DHTPIN, DHTTYPE);

// void setup()
// {
//   Serial.begin(115200);
//   delay(2000);
//   Serial.println("start");

//   Serial.println("\n=== ESP32-S3 LED Pin Scanner ===");
//   Serial.printf("Chip Model: %s\n", ESP.getChipModel());
//   Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
//   Serial.printf("PSRAM Size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
//   Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
//   Serial.printf("Total Heap: %d bytes\n", ESP.getHeapSize());
//   Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());

//   delay(1000);
//   dht.begin();
// }

// void loop()
// {
//   float humidity = dht.readHumidity();
//   float temperature = dht.readTemperature(); // Celsius

//   if (isnan(humidity) || isnan(temperature)) {
//     Serial.println("Failed to read from DHT sensor!");
//     delay(2000);
//     return;
//   }

//   Serial.print("Temperature: ");
//   Serial.print(temperature);
//   Serial.print(" °C  |  Humidity:");
//   Serial.print(humidity);
//   Serial.println(" %");

//   delay(2000); // Wait 2 seconds before next read

//   return;
// }


// #include <Adafruit_NeoPixel.h>

// #define LED_PIN 48
// #define NUM_LEDS 1

// Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// void setup()
// {
// Serial.begin(115200);
// delay(2000);
// Serial.println("start");

// Serial.println("\n=== ESP32-S3 LED Pin Scanner ===");
// Serial.printf("Chip Model: %s\n", ESP.getChipModel());
// Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
// Serial.printf("PSRAM Size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
// Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
// Serial.printf("Total Heap: %d bytes\n", ESP.getHeapSize());
// Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());

// strip.setBrightness(10); 

// strip.begin(); // Initialize
// strip.show(); // Turn OFF all pixels initially
// delay(1000);
// }

// void loop()
// {
// // Cycle colors
// strip.setPixelColor(0, strip.Color(255, 0, 0)); // Red
// strip.show();
// delay(1000);

// strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green
// strip.show();
// delay(1000);

// strip.setPixelColor(0, strip.Color(0, 0, 255)); // Blue
// strip.show();
// delay(1000);

// strip.setPixelColor(0, strip.Color(255, 255, 255)); // White
// strip.show();
// delay(1000);
  
// Serial.println("done");
// }

#include <WiFi.h>
#include <PubSubClient.h>

// ====== WiFi ======
const char* WIFI_SSID     = "Jovanovic";
const char* WIFI_PASSWORD = "";

// ====== MQTT Broker (Raspberry Pi) ======
const char* MQTT_HOST = "192.168.0.17";   // <-- Replace with your Pi's IP
const uint16_t MQTT_PORT = 1883;          // Default MQTT port (no TLS)

// MQTT Topics
const char* PUB_TOPIC = "sensors/esp32s3/temperature";
const char* SUB_TOPIC = "sensors/esp32s3/cmd";  // optional

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

unsigned long lastPublish = 0;

// Handle incoming MQTT messages
void onMqttMessage(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("]: ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

// Connect to Wi-Fi
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

// Connect to MQTT broker
void connectMQTT() {
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);

  while (!mqtt.connected()) {
    String clientId = "esp32s3-" + String((uint32_t)ESP.getEfuseMac(), HEX);
    Serial.printf("Connecting to MQTT as %s ...\n", clientId.c_str());
    if (mqtt.connect(clientId.c_str())) {
      Serial.println("MQTT connected!");
      mqtt.subscribe(SUB_TOPIC);
    } else {
      Serial.printf("MQTT connection failed, rc=%d. Retrying in 2s...\n", mqtt.state());
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
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqtt.connected()) connectMQTT();
  mqtt.loop();

  // Publish every 5 seconds
  unsigned long now = millis();
  if (now - lastPublish >= 5000) {
    lastPublish = now;

    float temperature = 25.0 + (float)(millis() % 1000) / 100.0;  // fake data
    char payload[64];
    snprintf(payload, sizeof(payload), "{\"temperature\": %.2f}", temperature);

    bool success = mqtt.publish(PUB_TOPIC, payload);
    Serial.printf("Publish %s: %s\n", success ? "OK" : "FAILED", payload);
  }
}
