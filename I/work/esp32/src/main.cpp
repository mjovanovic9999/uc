#include <edge-impulse-sdk/classifier/ei_run_classifier.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2s.h"
#include <Adafruit_NeoPixel.h>

#define I2S_WS 14
#define I2S_SD 13
#define I2S_SCK 12
#define SAMPLE_RATE 16000

#define MQ2_PIN 1
#define LED_PIN 16

#define RGB_PIN 48

Adafruit_NeoPixel strip(1, RGB_PIN, NEO_GRB + NEO_KHZ800);

float sensorPercentage;
bool shouldReadSensor = false;

static void readMQ2()
{
    float sensorValue = analogRead(MQ2_PIN);

    float voltage = (sensorValue / 4095.0) * 2.5;
    Serial.print("MQ-2 Percentage: ");
    sensorPercentage = voltage / 2.5 * 100;
    Serial.print(sensorPercentage);
    Serial.print("% | Voltage: ");
    Serial.println(voltage, 2);
}

static void turnOnLED()
{
    digitalWrite(LED_PIN, HIGH);
}

static void turnOffLED()
{
    digitalWrite(LED_PIN, LOW);
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

    Serial.println("Edge Impulse Inferencing Demo");

    ei_printf("Inferencing settings:\n");
    ei_printf("\tInterval: %.2f ms.\n", (float)EI_CLASSIFIER_INTERVAL_MS);
    ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
    ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
    ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

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
    ei_printf("Recording...\n");

    pinMode(LED_PIN, OUTPUT);
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
        ei_printf("Predictions ");
        ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
                  result.timing.dsp, result.timing.classification, result.timing.anomaly);
        ei_printf(": \n");
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
        {
            ei_printf("    %s: ", result.classification[ix].label);
            ei_printf_float(result.classification[ix].value);
            ei_printf("\n");

            if (result.classification[ix].value > 0.8)
            {
                if (strcmp(result.classification[ix].label, "go") == 0)
                {
                    ei_printf(">>> DETECTED: go <<<\n");
                    strip.setPixelColor(0, strip.Color(0, 255, 0));
                    strip.show();
                    shouldReadSensor = true;
                }
                else if (strcmp(result.classification[ix].label, "stop") == 0)
                {
                    ei_printf(">>> DETECTED: stop <<<\n");
                    strip.setPixelColor(0, strip.Color(255, 0, 0));
                    strip.show();
                    shouldReadSensor = false;
                }
                else if (strcmp(result.classification[ix].label, "forward") == 0)
                {
                    ei_printf(">>> DETECTED: forward <<<\n");
                    strip.setPixelColor(0, strip.Color(255, 255, 255));
                    strip.show();
                    shouldReadSensor = false;
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
        if (sensorPercentage > 20.0)
            turnOnLED();
        else
            turnOffLED();
    }
    else
    {
        turnOffLED();
    }
}