#include <RH_ASK.h>
#include <SPI.h> // Not actualy used but needed to compile

// tx/rx configuration
const int txSpeed = 2000;
const int rxPin = 11;
const int txPin = 12;
const int pttPin = 7;

RH_ASK driver(txSpeed, rxPin, txPin, pttPin);

void setup() {
  Serial.begin(9600);
  Serial.println("==> Initialise receiver...");
  if (!driver.init()) {
    Serial.println("Initialisation failed");
  } else {
    Serial.println("Initialisation complete");
  }
}

void loop() {
  float buf[4] = {0};
  uint8_t buflen = sizeof(buf);
  
  if (driver.recv((uint8_t*)buf, &buflen)) {
    int i;
    char message[100];
    sprintf(message, "+++ New message received :: %.2f ºC :: %f %% humidity :: Feels like %f ºC :: %f Volts", (float)buf[0], (float)buf[1], (float)buf[2], (float)buf[3]);
    Serial.println(message);
    // Message with a good checksum received, dump it.
//    Serial.printf(F("+++ New message received :: %.2f ºC :: %d %% humidity :: Feels like %.2f ºC :: %.2f Volts\n"), buf[0], buf[1], buf[2], buf[3]);
    Serial.print("+++ New message received!");
    Serial.print("-> Raw data: ");
    Serial.println((char*)buf);

    float temperature = buf[0];
    float humidity = buf[1];
    float humiture = buf[2];
    float voltage = buf[2];
    
    Serial.println("-> Parsed data:");
    Serial.print("+-> Temperature: ");
    Serial.println(temperature);
    Serial.print("+-> Humidity: ");
    Serial.println(humidity);
    Serial.print("+-> Humiture: ");
    Serial.println(humiture);
    Serial.print("+-> Voltage: ");
    Serial.println(humiture);
  }
}
