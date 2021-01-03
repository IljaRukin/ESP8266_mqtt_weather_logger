/*
 * connections
 * D1 - SCL
 * D2 - SDA
 * GND, VCC (3.3V)
*/

#include <Wire.h>
#include "cactus_io_BME280_I2C.h"
BME280_I2C bme(0x76);  // I2C using address 0x76

void setup() {
  Serial.begin(9600);
  bme.begin();
  bme.setTempCal(0);
}

void loop() {
  bme.readSensor();
  Serial.print("pressure: ");
  float pressure = bme.getPressure_MB();
  Serial.print(pressure); Serial.print(",");
  Serial.print("   humidity: ");
  float humidity = bme.getHumidity();
  Serial.print(humidity); Serial.print(",");
  Serial.print("   temperature: ");
  float temperature = bme.getTemperature_C();
  Serial.print(temperature); Serial.print(",");
  Serial.println("");
  delay(1000);
}
