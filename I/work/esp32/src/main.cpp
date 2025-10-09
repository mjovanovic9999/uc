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

#include <driver/i2s.h>
#include <Arduino.h>
// #include <edge-impulse-sdk/classifier/ei_run_classifier.h>

#define I2S_WS 14
#define I2S_SD 13
#define I2S_SCK 12
#define SAMPLE_RATE 16000

// WHOLE 24BITS recording

#define BUFFER_SIZE 512
int32_t i2s_buffer[BUFFER_SIZE];
void setup()
{
  Serial.begin(921600); // High baud rate for audio streaming

  // Configure I2S
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,
      .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 4,
      .dma_buf_len = BUFFER_SIZE,
      .use_apll = false,
      .tx_desc_auto_clear = false,
      .fixed_mclk = 0};

  // Pin configuration
  i2s_pin_config_t pin_config = {
      .bck_io_num = I2S_SCK,
      .ws_io_num = I2S_WS,
      .data_out_num = I2S_PIN_NO_CHANGE,
      .data_in_num = I2S_SD};

  // Install and start I2S driver
  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);
  i2s_zero_dma_buffer(I2S_NUM_0);

  delay(100);
}

void loop()
{
  size_t bytes_read = 0;

  // Read audio data from I2S
  i2s_read(I2S_NUM_0, i2s_buffer, sizeof(i2s_buffer), &bytes_read, portMAX_DELAY);

  // Convert 32-bit to 16-bit and send via serial
  int samples_read = bytes_read / sizeof(int32_t);
  for (int i = 0; i < samples_read; i++)
  {
    // Extract upper 16 bits (most significant)
    int16_t sample = ((i2s_buffer[i] >> 16) & 0xFFFF) << 1;
    Serial.write((uint8_t *)&sample, 2);
  }
}


// #include <Adafruit_NeoPixel.h>

// #define LED_PIN 48
// #define NUM_LEDS 1

// Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// // //BASED ON EXAMPLE
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"

// #include "driver/i2s.h"



// #define LEDPIN 16

// /** Audio buffers, pointers and selectors */
// typedef struct
// {
//   signed short *buffers[2];
//   unsigned char buf_select;
//   unsigned char buf_ready;
//   unsigned int buf_count;
//   unsigned int n_samples;
// } inference_t;

// static inference_t inference;
// static const uint32_t sample_buffer_size = 8192;
// static signed short sampleBuffer[sample_buffer_size];
// static bool debug_nn = false; // Set this to true to see e.g. features generated from the raw signal
// static int print_results = -(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW);
// static bool record_status = true;

// static void audio_inference_callback(uint32_t n_bytes)
// {
//   for (int i = 0; i < n_bytes >> 1; i++)
//   {
//     inference.buffers[inference.buf_select][inference.buf_count++] = sampleBuffer[i];

//     if (inference.buf_count >= inference.n_samples)
//     {
//       inference.buf_select ^= 1;
//       inference.buf_count = 0;
//       inference.buf_ready = 1;
//     }
//   }
// }

// static void capture_samples(void *arg)
// {

//   const int32_t i2s_bytes_to_read = (uint32_t)arg;
//   size_t bytes_read = i2s_bytes_to_read;

//   while (record_status)
//   {

//     /* read data at once from i2s */
//     i2s_read(I2S_NUM_0, (void *)sampleBuffer, i2s_bytes_to_read, &bytes_read, 100);

//     if (bytes_read <= 0)
//     {
//       ei_printf("Error in I2S read : %d", bytes_read);
//     }
//     else
//     {
//       if (bytes_read < i2s_bytes_to_read)
//       {
//         ei_printf("Partial I2S read");
//       }

//       // scale the data (otherwise the sound is too quiet)
//       for (int x = 0; x < i2s_bytes_to_read / 2; x++)
//       {
//         sampleBuffer[x] = (int16_t)(sampleBuffer[x]) * 8;
//       }

//       if (record_status)
//       {
//         audio_inference_callback(i2s_bytes_to_read);
//       }
//       else
//       {
//         break;
//       }
//     }
//   }
//   vTaskDelete(NULL);
// }

// /**
//  * @brief      Wait on new data
//  *
//  * @return     True when finished
//  */
// static bool microphone_inference_record(void)
// {
//   bool ret = true;

//   if (inference.buf_ready == 1)
//   {
//     ei_printf(
//         "Error sample buffer overrun. Decrease the number of slices per model window "
//         "(EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW)\n");
//     ret = false;
//   }

//   while (inference.buf_ready == 0)
//   {
//     delay(1);
//   }

//   inference.buf_ready = 0;
//   return true;
// }

// /**
//  * Get raw audio signal data
//  */
// static int microphone_audio_signal_get_data(size_t offset, size_t length, float *out_ptr)
// {
//   numpy::int16_to_float(&inference.buffers[inference.buf_select ^ 1][offset], out_ptr, length);

//   return 0;
// }

// static int i2s_init(uint32_t sampling_rate)
// {
//   // Start listening for audio: MONO @ 8/16KHz
//   i2s_config_t i2s_config = {
//       .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
//       .sample_rate = sampling_rate,
//       .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
//       .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
//       .communication_format = I2S_COMM_FORMAT_STAND_I2S,
//       .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
//       .dma_buf_count = 8,
//       .dma_buf_len = 512,
//       .use_apll = false,
//       .tx_desc_auto_clear = false,
//       .fixed_mclk = -1,
//   };
//   i2s_pin_config_t pin_config = {
//       .bck_io_num = I2S_SCK,
//       .ws_io_num = I2S_WS,
//       .data_out_num = I2S_PIN_NO_CHANGE,
//       .data_in_num = I2S_SD};
//   esp_err_t ret = 0;

//   ret = i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
//   if (ret != ESP_OK)
//   {
//     ei_printf("Error in i2s_driver_install");
//   }

//   ret = i2s_set_pin(I2S_NUM_0, &pin_config);
//   if (ret != ESP_OK)
//   {
//     ei_printf("Error in i2s_set_pin");
//   }

//   ret = i2s_zero_dma_buffer(I2S_NUM_0);
//   if (ret != ESP_OK)
//   {
//     ei_printf("Error in initializing dma buffer with 0");
//   }

//   return int(ret);
// }

// static int i2s_deinit(void)
// {
//   i2s_driver_uninstall(I2S_NUM_0); // stop & destroy i2s driver
//   return 0;
// }

// #if !defined(EI_CLASSIFIER_SENSOR) || EI_CLASSIFIER_SENSOR != EI_CLASSIFIER_SENSOR_MICROPHONE
// #error "Invalid model for current sensor."
// #endif

// /**
//  * @brief      Stop PDM and release buffers
//  */
// static void microphone_inference_end(void)
// {
//   i2s_deinit();
//   ei_free(inference.buffers[0]);
//   ei_free(inference.buffers[1]);
// }

// /**
//  * @brief      Init inferencing struct and setup/start PDM
//  *
//  * @param[in]  n_samples  The n samples
//  *
//  * @return     { description_of_the_return_value }
//  */
// static bool microphone_inference_start(uint32_t n_samples)
// {
//   inference.buffers[0] = (signed short *)malloc(n_samples * sizeof(signed short));

//   if (inference.buffers[0] == NULL)
//   {
//     return false;
//   }

//   inference.buffers[1] = (signed short *)malloc(n_samples * sizeof(signed short));

//   if (inference.buffers[1] == NULL)
//   {
//     ei_free(inference.buffers[0]);
//     return false;
//   }

//   inference.buf_select = 0;
//   inference.buf_count = 0;
//   inference.n_samples = n_samples;
//   inference.buf_ready = 0;

//   if (i2s_init(EI_CLASSIFIER_FREQUENCY))
//   {
//     ei_printf("Failed to start I2S!");
//   }

//   ei_sleep(100);

//   record_status = true;

//   xTaskCreate(capture_samples, "CaptureSamples", 1024 * 32, (void *)sample_buffer_size, 10, NULL);

//   return true;
// }

// /**
//  * @brief      Arduino setup function
//  */
// void setup()
// {
//   // put your setup code here, to run once:
//   Serial.begin(115200);
//   // comment out the below line to cancel the wait for USB connection (needed for native USB)
//   while (!Serial)
//     ;
//   Serial.println("Edge Impulse Inferencing Demo");

//   // summary of inferencing settings (from model_metadata.h)
//   ei_printf("Inferencing settings:\n");
//   ei_printf("\tInterval: ");
//   ei_printf_float((float)EI_CLASSIFIER_INTERVAL_MS);
//   ei_printf(" ms.\n");
//   ei_printf("\tFrame size: %d\n", EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE);
//   ei_printf("\tSample length: %d ms.\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT / 16);
//   ei_printf("\tNo. of classes: %d\n", sizeof(ei_classifier_inferencing_categories) / sizeof(ei_classifier_inferencing_categories[0]));

//   run_classifier_init();
//   ei_printf("\nStarting continious inference in 2 seconds...\n");
//   ei_sleep(2000);

//   if (microphone_inference_start(EI_CLASSIFIER_SLICE_SIZE) == false)
//   {
//     ei_printf("ERR: Could not allocate audio buffer (size %d), this could be due to the window length of your model\r\n", EI_CLASSIFIER_RAW_SAMPLE_COUNT);
//     return;
//   }
//   strip.setBrightness(10);
//   strip.begin();
//   strip.show();
//   ei_printf("Recording...\n");


//   //  pinMode(LEDPIN, OUTPUT);
//   //  digitalWrite(LEDPIN, HIGH);
// }

// /**
//  * @brief      Arduino main function. Runs the inferencing loop.
//  */
// void loop()
// {
//   bool m = microphone_inference_record();
//   if (!m)
//   {
//     ei_printf("ERR: Failed to record audio...\n");
//     return;
//   }

//   signal_t signal;
//   signal.total_length = EI_CLASSIFIER_SLICE_SIZE;
//   signal.get_data = &microphone_audio_signal_get_data;
//   ei_impulse_result_t result = {0};

//   EI_IMPULSE_ERROR r = run_classifier_continuous(&signal, &result, debug_nn);
//   if (r != EI_IMPULSE_OK)
//   {
//     ei_printf("ERR: Failed to run classifier (%d)\n", r);
//     return;
//   }

//   if (++print_results >= (EI_CLASSIFIER_SLICES_PER_MODEL_WINDOW))
//   {
//     // // print the predictions
//     // ei_printf("Predictions ");
//     // ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
//     //           result.timing.dsp, result.timing.classification, result.timing.anomaly);
//     // ei_printf(": \n");
//     // for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
//     // {
//     //   ei_printf("    %s: ", result.classification[ix].label);
//     //   ei_printf_float(result.classification[ix].value);
//     //   ei_printf("\n");
//     for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++)
//     {
//       if (result.classification[ix].value > 0.8)
//       {
//         // Print all results
//         for (size_t j = 0; j < EI_CLASSIFIER_LABEL_COUNT; j++)
//         {
//           ei_printf("    %s: %.2f\n",
//                     result.classification[j].label,
//                     result.classification[j].value);
//         }

//         ei_printf(">>> DETECTED: %s <<<\n", result.classification[ix].label);

//         if (strcmp(result.classification[ix].label, "go") == 0)
//         {
//           strip.setPixelColor(0, strip.Color(0, 255, 0));
//           strip.show();
//         }
//         else if (strcmp(result.classification[ix].label, "stop") == 0)
//         {
//           strip.setPixelColor(0, strip.Color(255, 0, 0));
//           strip.show();
//         }
//         else if (strcmp(result.classification[ix].label, "forward") == 0)
//         {
//           strip.setPixelColor(0, strip.Color(255, 255, 255));
//           strip.show();
//         }
//       }
//     }

// #if EI_CLASSIFIER_HAS_ANOMALY == 1
//     ei_printf("    anomaly score: ");
//     ei_printf_float(result.anomaly);
//     ei_printf("\n");
// #endif

//     print_results = 0;
//   }
// }
