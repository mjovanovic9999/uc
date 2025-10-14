#include "DHT.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <edge-impulse-sdk/classifier/ei_run_classifier.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include <Adafruit_NeoPixel.h>
#include <ArduinoJson.h>

#define I2S_WS 14
#define I2S_SD 13
#define I2S_SCK 12
#define SAMPLE_RATE 16000

#define MQ2_PIN 1
#define LED_PIN 16
#define RGB_PIN 48
#define DHT_PIN 4

#define LED_HUMIDITY_PIN 39
#define LED_LEAKAGE_PIN 41
#define LED_FIRE_PIN 2

const char *WIFI_SSID = "test";
const char *WIFI_PASSWORD = "test1234";

const char *MQTT_HOST = "10.224.221.14";
const uint16_t MQTT_PORT = 1883;

const char *PUB_MQ2_TOPIC = "sensors/mq2";
const char *PUB_DHT11_TOPIC = "sensors/dht11";
const char *SUB_ALERTS_TOPIC = "alerts";

const char *PUB_COMMANDS_TOPIC = "esp32/status";
const char *SUB_COMMANDS_TOPIC = "esp32/commands";

const char *CLIENT_ID = "esp32-s3";

unsigned long lastPublish = 0;
bool shouldReadSensor = false;
bool shouldForward = false;
float airQuality;
float humidity;
float temperature;

Adafruit_NeoPixel strip(1, RGB_PIN, NEO_GRB + NEO_KHZ800);

WiFiClient wifiClient;
PubSubClient mqtt(wifiClient);

DHT dht(DHT_PIN, DHT11);

static void readDHT11()
{
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C  |  Humidity:");
  Serial.print(humidity);
  Serial.println(" %");
}

static void readMQ2()
{
  float sensorValue = analogRead(MQ2_PIN);

  float voltage = (sensorValue / 4095.0) * 2.5;
  Serial.print("MQ-2 Percentage: ");
  airQuality = voltage / 2.5 * 100;
  Serial.print(airQuality);
  Serial.print("% | Voltage: ");
  Serial.println(voltage, 2);
}

void connectWiFi()
{
  Serial.printf("Connecting to WiFi: %s\n", WIFI_SSID);

  WiFi.mode(WIFI_STA);
  delay(1000);

  WiFi.setTxPower(WIFI_POWER_5dBm);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print('.');
  }

  Serial.println("Connecting");
  Serial.printf("\nWiFi connected!\n");
  Serial.printf("IP: %s\n", WiFi.localIP().toString().c_str());
  Serial.printf("Signal: %d dBm\n", WiFi.RSSI());
}

void onMqttMessage(char *topic, byte *payload, unsigned int length)
{
  String message;
  for (unsigned int i = 0; i < length; i++)
  {
    message += (char)payload[i];
  }
  Serial.print("MQTT message [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.println(message);

  StaticJsonDocument<200> doc;
  DeserializationError error = deserializeJson(doc, message);
  if (error)
  {
    Serial.println("Failed to parse JSON");
    return;
  }

  const char *type = doc["type"];

  digitalWrite(LED_HUMIDITY_PIN, LOW);
  digitalWrite(LED_LEAKAGE_PIN, LOW);
  digitalWrite(LED_FIRE_PIN, LOW);

  if (strcmp(type, "gas_leak") == 0)
  {
    digitalWrite(LED_LEAKAGE_PIN, HIGH);
  }
  else if (strcmp(type, "fire") == 0)
  {
    digitalWrite(LED_FIRE_PIN, HIGH);
  }
  else if (strcmp(type, "high_humidity") == 0)
  {
    digitalWrite(LED_HUMIDITY_PIN, HIGH);
  }
}

void connectMQTT()
{
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqttMessage);

  while (!mqtt.connected())
  {
    if (mqtt.connect("esp32s3"))
    {
      Serial.println("MQTT connected!");
      mqtt.subscribe(SUB_ALERTS_TOPIC);
    }
    else
    {
      Serial.printf("MQTT connection failed, rc=%d. Retrying...\n", mqtt.state());
      delay(500);
    }
  }
}

/** Audio buffers, pointers and selectors */
typedef struct
{
  signed short *buffers[2];
  unsigned char buf_select;
  unsigned char buf_ready;
  unsigned int buf_count;
  unsigned int n_samples;
} inference_t;

static inference_t inference;
static const uint32_t sample_buffer_size = 2048;
static int32_t sampleBuffer[sample_buffer_size];
static bool debug_nn = false;
static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);
static bool record_status = true;

static void audio_inference_callback(uint32_t n_bytes)
{
  for (int i = 0; i < n_bytes / 4; i++)
  {
    // Use the same conversion as your working code
    int16_t sample16 = (int16_t)((sampleBuffer[i] >> 16) & 0xFFFF) << 1;

    inference.buffers[inference.buf_select][inference.buf_count++] = sample16;

    if (inference.buf_count >= inference.n_samples)
    {
      inference.buf_select ^= 1;
      inference.buf_count = 0;
      inference.buf_ready = 1;
    }
  }
}

static void capture_samples(void *arg)
{
  const int32_t i2s_bytes_to_read = (uint32_t)arg;
  size_t bytes_read = i2s_bytes_to_read;

  while (record_status)
  {
    i2s_read(I2S_NUM_0, (void *)sampleBuffer, i2s_bytes_to_read, &bytes_read, 100);

    if (bytes_read <= 0)
    {
      ei_printf("Error in I2S read: %d\n", bytes_read);
    }
    else
    {
      if (bytes_read < i2s_bytes_to_read)
      {
        ei_printf("Partial I2S read: %d vs %d\n", bytes_read, i2s_bytes_to_read);
      }

      if (record_status)
      {
        audio_inference_callback(bytes_read);
      }
      else
      {
        break;
      }
    }
  }
  vTaskDelete(NULL);
}

static bool microphone_inference_record(void)
{
  if (inference.buf_ready == 1)
  {
    ei_printf("Error sample buffer overrun. Decrease EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW\n");
    return false;
  }

  while (inference.buf_ready == 0)
  {
    delay(1);
  }

  inference.buf_ready = 0;
  return true;
}

static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
{
  numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);
  return 0;
}

static int i2s_init(uint32_t sampling_rate)
{
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = sampling_rate,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = 0,
      .dma_buf_count = 8,
      .dma_buf_len = 512,
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = -1};

  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_SD};

  esp_err_t ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  if (ret != ESP_OK)
  {
    ei_printf("Error in i2s_driver_install: %d\n", ret);
    return 1;
  }

  ret = i2s_set_pin(I2S_NUM_0, &pin_config);
  if (ret != ESP_OK)
  {
    ei_printf("Error in i2s_set_pin: %d\n", ret);
    return 1;
  }

  ret = i2s_zero_dma_buffer(I2S_NUM_0);
  if (ret != ESP_OK)
  {
    ei_printf("Error zeroing DMA buffer: %d\n", ret);
  }

  return 0;
}

static int i2s_deinit(void)
{
  i2s_driver_uninstall(I2S_NUM_0);
  return 0;
}

static bool microphone_inference_start(uint32_t n_samples)
{
  inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));
  inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));

  if (inference.buffers[0] == NULL || inference.buffers[1] == NULL)
  {
    ei_printf("Failed to allocate inference buffers\n");
    return false;
  }

  inference.buf_select = 0;
  inference.buf_count = 0;
  inference.n_samples = n_samples;
  inference.buf_ready = 0;

  if (i2s_init(EI_CLASSIFIER_FREQUENCY))
  {
    ei_printf("Failed to start I2S!\n");
    return false;
  }

  ei_sleep(100);
  record_status = true;

  xTaskCreate(capture_samples, "CaptureSamples", 1024 * 32,
              (void *)(sample_buffer_size * sizeof(int32_t)), 10, NULL);

  return true;
}

static void microphone_inference_end(void)
{
  i2s_deinit();
  free(inference.buffers[0]);
  free(inference.buffers[1]);
}

#if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
#error "Invalid model for current sensor."
#endif

void setup()
{
  Serial.begin(115200);
  analogReadResolution(12);
  while (!Serial)
    ;

  connectWiFi();
  connectMQTT();
  dht.begin();

  run_classifier_init();
  ei_printf("\nStarting continuous inference in 2 seconds...\n");
  ei_sleep(2000);

  if (!microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE))
  {
    ei_printf("ERR: Could not allocate audio buffer\n");
    return;
  }

  strip.setBrightness(10);
  strip.begin();
  strip.show();

  pinMode(LED_PIN, OUTPUT);
  pinMode(LED_FIRE_PIN, OUTPUT);
  pinMode(LED_LEAKAGE_PIN, OUTPUT);
  pinMode(LED_HUMIDITY_PIN, OUTPUT);

  ei_printf("Recording...\n");
}

void loop()
{
  if (!microphone_inference_record())
  {
    ei_printf("ERR: Failed to record audio\n");
    return;
  }

  signal_t signal;
  signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
  signal.get_data = &microphone_audio_signal_get_data;
  ei_impulse_result_t result = {0};

  EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);
  if (r != EI_IMPULSE_OK)
  {
    ei_printf("ERR: Failed to run classifier (%d)\n", r);
    return;
  }

  if (++print_results >= (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW))
  {
    // ei_printf("Predictions ");
    // ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
    //           result.timing.dsp, result.timing.classification, result.timing.anomaly);
    // ei_printf(": \n");
    for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
    {
      // ei_printf("    %s: ", result.classification[ix].label);
      // ei_printf_float(result.classification[ix].value);
      // ei_printf("\n");

      if (result.classification[ix].value > 0.85)
      {
        if (strcmp(result.classification[ix].label, "go") == 0)
        {
          ei_printf(">>> DETECTED: go <<<\n");
          strip.setPixelColor(0, strip.Color(0, 255, 0));
          strip.show();
          shouldReadSensor = true;
          shouldForward = false;
        }
        else if (strcmp(result.classification[ix].label, "stop") == 0)
        {
          ei_printf(">>> DETECTED: stop <<<\n");
          strip.setPixelColor(0, strip.Color(255, 0, 0));
          strip.show();
          shouldReadSensor = false;
          shouldForward = false;
        }
        else if (strcmp(result.classification[ix].label, "forward") == 0)
        {
          ei_printf(">>> DETECTED: forward <<<\n");
          strip.setPixelColor(0, strip.Color(255, 255, 255));
          strip.show();
          shouldReadSensor = true;
          shouldForward = true;
        }
      }
    }

#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("    anomaly score: %.3f\n", result.anomaly);
#endif

    print_results = 0;
  }

  if (shouldReadSensor)
  {
    readMQ2();
    readDHT11();
    if (airQuality > 20.0)
      digitalWrite(LED_PIN, HIGH);
    else
      digitalWrite(LED_PIN, LOW);
  }
  else
  {
    digitalWrite(LED_PIN, LOW);
  }

  if (WiFi.status() != WL_CONNECTED)
    connectWiFi();
  if (!mqtt.connected())
    connectMQTT();

  if (shouldForward)
  {
    unsigned long now = millis();
    if (now - lastPublish >= 5000)
    {
      lastPublish = now;

      char payload_dht11[64];
      snprintf(payload_dht11, sizeof(payload_dht11), "{\"temperature\": %.2f,\"humidity\": %.2f}", temperature, humidity);

      bool success = mqtt.publish(PUB_DHT11_TOPIC, payload_dht11, true);
      Serial.printf("Publish %s: %s\n", success ? "OK" : "FAILED", payload_dht11);

      char payload_mq2[64];
      snprintf(payload_mq2, sizeof(payload_mq2), "{\"quality\": %.2f}", airQuality);

      success = mqtt.publish(PUB_MQ2_TOPIC, payload_mq2, true);
      Serial.printf("Publish %s: %s\n", success ? "OK" : "FAILED", payload_mq2);
    }
  }

  mqtt.loop();
}
