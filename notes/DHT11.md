```cpp
//adafruit/DHT sensor library @ ^1.4.4
//adafruit/Adafruit Unified Sensor @ ^1.1.14

#ifndef DHTSENSOR_H
#define DHTSENSOR_H

#include <Arduino.h>
#include "DHT.h"

// === Function Prototypes ===
void initDHTSensor();
void readDHTSensor();

#endif
```

```cpp
#include "DHTSensor.h"

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void initDHTSensor()
{
  delay(1000);
  dht.begin();
}

void readDHTSensor()
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();     // Celsius

  if (isnan(humidity) || isnan(temperature))
  {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C  |  Humidity: ");
  Serial.print(humidity);
  Serial.println(" %");
}

```

### FULL
```cpp
#include <Arduino.h>
#include "DHT.h"

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

void setup()
{
  Serial.begin(115200);
  delay(2000);
  Serial.println("start");

  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Flash Size: %d MB\n", ESP.getFlashChipSize() / (1024 * 1024));
  Serial.printf("PSRAM Size: %d MB\n", ESP.getPsramSize() / (1024 * 1024));
  Serial.printf("Free PSRAM: %d bytes\n", ESP.getFreePsram());
  Serial.printf("Total Heap: %d bytes\n", ESP.getHeapSize());
  Serial.printf("Free Heap: %d bytes\n", ESP.getFreeHeap());

  delay(1000);
  dht.begin();
}

void loop()
{
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature(); // Celsius

  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
    delay(2000);
    return;
  }

  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" °C  |  Humidity:");
  Serial.print(humidity);
  Serial.println(" %");

  delay(2000); // Wait 2 seconds before next read

  return;
}
```