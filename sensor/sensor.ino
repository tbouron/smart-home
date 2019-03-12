#include <DHT.h>
#include <RH_ASK.h>
#include <SPI.h>
#include <epd1in54.h>
#include <epdpaint.h>
//#include <LowPower.h>
#include "DotFont.h"

// DHT configuration
#define DHTPIN 5
#define DHTTYPE DHT22
// TX/TX configuration
#define TXVCCPIN 2
#define TXSPEED 2000
#define RXPIN 4
#define TXPIN 3
#define PTTPIN 7
// E-paper configuration
#define COLORED 0
#define UNCOLORED 1

const DHT dht(DHTPIN, DHTTYPE);
const RH_ASK driver(TXSPEED, RXPIN, TXPIN, PTTPIN);

const unsigned char image[640];
const Paint paint(image, 0, 0);
const Epd epd;
const DotFont dotFont(&paint);
const int INTERVAL = 6;
const int ITERATIONS_BEFORE_SEND = 10;

int count = 0;

void setup() {
  Serial.begin(9600);
  Serial.println(F("==> Initialise sensor..."));
  
  dht.begin();

  pinMode(TXVCCPIN, OUTPUT);
  digitalWrite(TXVCCPIN, HIGH);
  if (!driver.init()) {
    Serial.println(F("TX initialisation failed"));
    return;
  }
  digitalWrite(TXVCCPIN, LOW);

  if (epd.Init(lut_full_update) != 0) {
    Serial.println(F("E-paper screen initialisation failed"));
    return;
  }

  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();

  if (epd.Init(lut_partial_update) != 0) {
    Serial.println(F("E-paper screen initialisation failed"));
    return;
  }
  
  paint.SetRotate(ROTATE_0);
  
  Serial.println(F("Initialisation complete"));
}

void loop() {
  Serial.println(F("==> Start loop..."));

  count++;

  float data[4];

  if (dht.read(true)) {
    // Reading temperature or humidity takes about 250 milliseconds!
    // Read temperature as Celsius (the default)
    data[0] = dht.readTemperature();
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    data[1] = dht.readHumidity();
    // Read heat index as Celsius (the default)
    data[2] = dht.computeHeatIndex(data[0], data[1], false);
  } else {
    data[0] = sqrt(-1);
    data[1] = sqrt(-1);
    data[2] = sqrt(-1);
  }

  if (isnan(data[0]) || isnan(data[1]) || isnan(data[2])) {
    Serial.println("[x] Failed to read from DHT sensor");
  }

  // Read the current voltage
  data[3] = readVcc();

  draw(data);

  Serial.print(F("Data collected =>"));
  Serial.print(F(" temparature :: "));
  Serial.print(data[0]);
  Serial.print(F(" | humidity :: "));
  Serial.print(data[1]);
  Serial.print(F(" | humiture :: "));
  Serial.print(data[2]);
  Serial.print(F(" | voltage :: "));
  Serial.print(data[3]);
  Serial.println();

  if (count % ITERATIONS_BEFORE_SEND == 0) {
    digitalWrite(TXVCCPIN, HIGH);
    if (driver.send((uint8_t *)data, 4 * sizeof(data[0]))) {
      driver.waitPacketSent();
      Serial.println(F("Payload sent"));
    } else {
      Serial.println(F("[x] Failed to send payload. Data or length invalid"));
    }
    digitalWrite(TXVCCPIN, LOW);
  }

//  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
//  LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
//                SPI_OFF, USART0_OFF, TWI_OFF);
  delay(INTERVAL * 1000);
}

//void initBoard() {
//  // The following saves some extra power by disabling some 
//  // peripherals I am not using.
//  
//  // Disable the ADC by setting the ADEN bit (bit 7) to zero.
//  ADCSRA = ADCSRA & B01111111;
//  
//  // Disable the analog comparator by setting the ACD bit
//  // (bit 7) to one.
//  ACSR = B10000000;
//  
//  // Disable digital input buffers on all analog input pins
//  // by setting bits 0-5 to one.
//  DIDR0 = DIDR0 | B00111111;
//}

void draw(float data[]) {
  epd.Reset();
  drawTemperature(data[0], data[2]);
  drawHumidity(data[1]);
  drawBatteryIndicator(data[3]);
  epd.DisplayFrame();
  epd.Sleep();
}

void drawTemperature(float temperature, float humiture) {
  char it[2] = "--";
  char dt[2] = "--";
  if (!isnan(temperature)) {
    char t[4];
    dtostrf(temperature, 4, 1, t);
    memcpy(it, t, 2);
    memcpy(dt, t+2, 2);
  }

  // Integral part of the temperature
  paint.SetWidth(100);
  paint.SetHeight(65);
  paint.Clear(UNCOLORED);
  dotFont.DrawCharAt(3, 3, it, 3);
  epd.SetFrameMemory(paint.GetImage(), 30, 20, paint.GetWidth(), paint.GetHeight());

  // Decimal part of the temperature
  paint.SetWidth(20);
  paint.SetHeight(25);
  paint.Clear(UNCOLORED);
  dotFont.DrawCharAt(1, 1, dt, 1);
  epd.SetFrameMemory(paint.GetImage(), 155, 60, paint.GetWidth(), paint.GetHeight());
  
  // Celcius sign
  paint.SetWidth(31);
  paint.SetHeight(31);
  paint.Clear(COLORED);
  paint.Clear(UNCOLORED);
  paint.DrawCircle(3, 3, 3, COLORED);
  paint.DrawHorizontalLine(15, 0, 15, COLORED);
  paint.DrawHorizontalLine(15, 30, 15, COLORED);
  paint.DrawVerticalLine(15, 0, 30, COLORED);  
  epd.SetFrameMemory(paint.GetImage(), 150, 20, paint.GetWidth(), paint.GetHeight());

  if (!isnan(humiture)) {
    char h[4];
    dtostrf(humiture, 4, 1, h);

    // Heat index
    paint.SetWidth(110);
    paint.SetHeight(16);
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(0, 0, "Feels like", &Font12, COLORED);
    epd.SetFrameMemory(paint.GetImage(), 30, 92, paint.GetWidth(), paint.GetHeight());

    paint.SetWidth(30);
    paint.SetHeight(10);
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(0, 0, h, &Font12, COLORED);
    epd.SetFrameMemory(paint.GetImage(), 106, 92, paint.GetWidth(), paint.GetHeight());
  }
}

void drawHumidity(float humidity) {
  char ih[2] = "--";
  if (!isnan(humidity)) {
    dtostrf(humidity, 2, 0, ih);
  }

  // Humidity
  paint.SetWidth(100);
  paint.SetHeight(65);
  paint.Clear(UNCOLORED);
  dotFont.DrawCharAt(3, 3, ih, 3);
  epd.SetFrameMemory(paint.GetImage(), 30, 120, paint.GetWidth(), paint.GetHeight());
  
  // Percent sign
  paint.SetWidth(20);
  paint.SetHeight(31);
  paint.Clear(UNCOLORED);
  paint.DrawFilledCircle(2, 2, 2, COLORED);
  paint.DrawFilledCircle(18, 28, 2, COLORED);
  paint.DrawLine(0, 30, 20, 0, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 155, 150, paint.GetWidth(), paint.GetHeight());

  if (!isnan(humidity)) {
    char rate[5];
    if (humidity < 30) {
      strncpy(rate, "HELP!", 5);
    } else if (humidity < 45) {
      strncpy(rate, "MEH..", 5);
    } else if (humidity < 55) {
      strncpy(rate, "YAY!", 5);
    } else if (humidity < 60) {
      strncpy(rate, "MEH..", 5);
    } else {
      strncpy(rate, "HELP!", 5);
    }

    // Draw the rate
    paint.SetWidth(30);
    paint.SetHeight(16);
    paint.Clear(UNCOLORED);
    paint.DrawStringAt(0, 0, rate, &Font12, COLORED);
    epd.SetFrameMemory(paint.GetImage(), 155, 120, paint.GetWidth(), paint.GetHeight());
  }
}

void drawBatteryIndicator(float voltage) {
  if (voltage <= 3.4) {
    paint.SetWidth(20);
    paint.SetHeight(11);
    paint.Clear(UNCOLORED);
    paint.DrawHorizontalLine(0, 0, 18, COLORED);
    paint.DrawHorizontalLine(0, 10, 18, COLORED);
    paint.DrawVerticalLine(0, 0, 10, COLORED);  
    paint.DrawVerticalLine(18, 0, 10, COLORED); 
    paint.DrawFilledRectangle(18, 2, 20, 8, COLORED); 
    epd.SetFrameMemory(paint.GetImage(), 155, 92, paint.GetWidth(), paint.GetHeight());
  }
}

//read internal voltage
float readVcc() {
  long result;
  // Read 1.1V reference against AVcc
  ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Convert
  while (bit_is_set(ADCSRA, ADSC));
  result = ADCL;
  result |= ADCH << 8;
  result = 1126400L / result; // Back-calculate AVcc in mV
  return float(double(result) / 1000);
}
