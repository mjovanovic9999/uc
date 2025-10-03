#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define NUM_LEDS 1

Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("start");

  Serial.println("\n=== ESP32-S3 LED Pin Scanner ===");
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("PSRAM Size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
  Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
  Serial.printf("Total Heap: %d bytes\n", ESP.getHeapSize());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());

  strip.setBrightness(10);

  strip.begin(); // Initialize
  strip.show();  // Turn OFF all pixels initially
  delay(1000);
}

void loop()
{
  // Cycle colors

  strip.setPixelColor(0, strip.Color(255, 0, 0)); // Red
  strip.show();
  delay(1000);

  strip.setPixelColor(0, strip.Color(0, 255, 0)); // Green
  strip.show();
  delay(1000);

  strip.setPixelColor(0, strip.Color(0, 0, 255)); // Blue
  strip.show();
  delay(1000);

  strip.setPixelColor(0, strip.Color(255, 255, 255)); // White
  strip.show();
  delay(1000);

  Serial.println("done");
}
