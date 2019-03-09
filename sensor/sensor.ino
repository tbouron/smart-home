#include <DHT.h>
#include <RH_ASK.h>
#include <SPI.h>
#include <epd1in54.h>
#include <epdpaint.h>
//#include <LowPower.h>
////#include "DotFont.h"
////#include "messages.h"

// DHT configuration
#define DHTPIN 2
#define DHTTYPE DHT22
// TX/TX configuration
#define TXSPEED 2000
#define RXPIN 11
#define TXPIN 12
#define PTTPIN 7
// E-paper configuration
#define COLORED 0
#define UNCOLORED 1

DHT dht(DHTPIN, DHTTYPE);
RH_ASK driver(TXSPEED, RXPIN, TXPIN, PTTPIN);

const unsigned char image[640];
const Paint paint(image, 0, 0);
const Epd epd;

const byte DOT_FONT[10][5] PROGMEM = {
  {
   0x7C,
   0x82,
   0x82,
   0x82,
   0x7C   
  },
  {
   0x00,
   0x00,
   0x00,
   0x02,
   0xFE   
  },
  {
   0xE4,
   0x92,
   0x92,
   0x92,
   0x4C  
  },
  {
   0x44,
   0x92,
   0x92,
   0x92,
   0x6C  
  }
,
  {
   0x1E,
   0x10,
   0x10,
   0x10,
   0xFE  
  }
,
  {
   0x5E,
   0x92,
   0x92,
   0x92,
   0x62  
  }
,
  {
   0x7C,
   0x92,
   0x92,
   0x92,
   0x64
  }
,
  {
   0x02,
   0xE2,
   0x12,
   0x0A,
   0x06
  }
,
  {
   0x6C,
   0x92,
   0x92,
   0x92,
   0x6C
  }
,
  {
   0x4C,
   0x92,
   0x92,
   0x92,
   0x7C
  }
};

void setup() {
  Serial.begin(9600);
  Serial.println("==> Initialise sensor...");

//  initBoard();
  
  dht.begin();

  if (!driver.init()) {
    Serial.println("TX initialisation failed");
    return;
  }

  if (epd.Init(lut_full_update) != 0) {
    Serial.println("E-paper screen initialisation failed");
    return;
  }

  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();
  epd.ClearFrameMemory(0xFF);   // bit set = white, bit reset = black
  epd.DisplayFrame();

  if (epd.Init(lut_partial_update) != 0) {
    Serial.println("E-paper screen initialisation failed");
    return;
  }
  
  paint.SetRotate(ROTATE_0);
  
  Serial.println("Initialisation complete");
}

void loop() {
  Serial.println("==> Start loop...");
  // Reading temperature or humidity takes about 250 milliseconds!
  // Read temperature as Celsius (the default)
  float data[4];
  data[0] = dht.readTemperature();
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  data[1] = dht.readHumidity();
  // Read heat index as Celsius (the default)
  data[2] = dht.computeHeatIndex(data[0], data[1], false);
  // Read the current voltage
  data[3] = readVcc();

  draw(data);

  Serial.println("Collected data:");
  Serial.print("-> temparature: ");
  Serial.println(data[0]);
  Serial.print("-> humidity: ");
  Serial.println(data[1]);
  Serial.print("-> humiture: ");
  Serial.println(data[2]);
  Serial.print("-> voltage: ");
  Serial.println(data[3]);
  
  // Check if any reads failed and exit early (to try again).
  if (isnan(data[0]) || isnan(data[1]) || isnan(data[2])) {
    Serial.println("[x] Failed to read from DHT sensor!");
  } else {
    Serial.println((char *)data);
    
    Serial.println("Sending payload...");
    driver.send((uint8_t *)data, 3 * sizeof(data[0]));
    driver.waitPacketSent();
  }

//  LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
//  LowPower.idle(SLEEP_8S, ADC_OFF, TIMER2_OFF, TIMER1_OFF, TIMER0_OFF, 
//                SPI_OFF, USART0_OFF, TWI_OFF);
//  delay(5000);
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

void circle_text(char txt[], int len, int x,int y, int radius){
  int left=x;
  for (int i=0; i<len; i++) {
    if (txt[i]>=48 and txt[i]<=57) {
      char digit[5];
      memcpy_P(digit, DOT_FONT[txt[i]-48], sizeof digit);
      draw_char(digit, left, y, radius);
      if (txt[i+1]==49) {
        left+=radius*10;
      } else {
        left+=radius*20;
      }
    } else if (txt[i]==58){
       paint.DrawFilledCircle(left, y+radius*15, radius, COLORED);
       paint.DrawFilledCircle(left, y+radius*4, radius, COLORED);
       left+=radius*8;
    } else if (txt[i]==46){
       paint.DrawFilledCircle(left, y+radius*18, radius, COLORED);
       left+=radius*8;
    } else if (txt[i] == 45) {
       paint.DrawFilledCircle(left+radius*9, y+radius*8, radius, COLORED);
       paint.DrawFilledCircle(left+radius*12, y+radius*8, radius, COLORED);
       left+=radius*20;
    } else {
       left+=radius*20;
    }  
  }
}

void draw_char(unsigned char c[5], int xbegin, int ybegin, byte radius) {  
//  epd_set_color(BLACK, WHITE);
  char ctemp;
  for (int x=0; x<5; x++){
    ctemp=c[x];
    for (int y=0; y<7; y++) {
      ctemp>>=1;
        if ( ctemp & 01) {
          paint.DrawFilledCircle(x*radius*3+xbegin, y*radius*3+ybegin, radius, COLORED);
        }
//        else
//          epd_draw_circle(x*radius*3+xbegin, y*radius*3+ybegin, radius);
    }
  }
}

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
//  PROGMEM
//  char it[2];
//  char dt[2];
//  strcpy_P(it, (char *)pgm_read_word(INIT)); 
//  strcpy_P(dt, (char *)pgm_read_word(INIT)); 
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
  circle_text(it, strlen(it), 3, 3, 3);
  epd.SetFrameMemory(paint.GetImage(), 30, 20, paint.GetWidth(), paint.GetHeight());

  // Decimal part of the temperature
  paint.SetWidth(20);
  paint.SetHeight(25);
  paint.Clear(UNCOLORED);
  circle_text(dt, strlen(dt), 1, 1, 1);
  epd.SetFrameMemory(paint.GetImage(), 155, 60, paint.GetWidth(), paint.GetHeight());

  char h[4] = "--";
//  PROGMEM
//  char h[4];
//  strcpy_P(h, (char *)pgm_read_word(MESSAGES[0])); 
  if (!isnan(humiture)) {
    dtostrf(humiture, 4, 1, h);
  }

  // Heat index
  paint.SetWidth(110);
  paint.SetHeight(16);
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(0, 0, "Feels like", &Font12, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 30, 92, paint.GetWidth(), paint.GetHeight());

  paint.SetWidth(30);
  paint.SetHeight(10);
  paint.Clear(COLORED);
  paint.DrawStringAt(0, 0, h, &Font12, UNCOLORED);
  epd.SetFrameMemory(paint.GetImage(), 106, 92, paint.GetWidth(), paint.GetHeight());
  
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
}

void drawHumidity(float humidity) {
  char ih[2] = "--";
//  PROGMEM
//  char ih[2];
//  strcpy_P(ih, (char *)pgm_read_word(MESSAGES[0])); 
  if (!isnan(humidity)) {
    dtostrf(humidity, 2, 0, ih);
  }

  // Humidity
  paint.SetWidth(100);
  paint.SetHeight(65);
  paint.Clear(UNCOLORED);
  circle_text(ih, strlen(ih), 3, 3, 3);
  epd.SetFrameMemory(paint.GetImage(), 30, 120, paint.GetWidth(), paint.GetHeight());
  
  // Percent sign
  paint.SetWidth(20);
  paint.SetHeight(31);
  paint.Clear(UNCOLORED);
  paint.DrawFilledCircle(2, 2, 2, COLORED);
  paint.DrawFilledCircle(18, 28, 2, COLORED);
  paint.DrawLine(0, 30, 20, 0, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 155, 150, paint.GetWidth(), paint.GetHeight());

  char rate[5];
  if (isnan(humidity)) {
    strncpy(rate, "", 1);
  } else if (humidity < 30) {
//    strcpy_P(rate, (char *)pgm_read_word(MESSAGES[1])); 
    strncpy(rate, "HELP!", 5);
  } else if (humidity < 45) {
//    strcpy_P(rate, (char *)pgm_read_word(MESSAGES[2])); 
    strncpy(rate, "MEH..", 5);
  } else if (humidity < 55) {
//    strcpy_P(rate, (char *)pgm_read_word(MESSAGES[3])); 
    strncpy(rate, "YAY! ", 5);
  } else if (humidity < 60) {
//    strcpy_P(rate, (char *)pgm_read_word(MESSAGES[1])); 
    strncpy(rate, "MEH..", 5);
  } else {
//    strcpy_P(rate, (char *)pgm_read_word(MESSAGES[2])); 
    strncpy(rate, "HELP!", 5);
  }

  // Draw the rate
  paint.SetWidth(30);
  paint.SetHeight(16);
  paint.Clear(UNCOLORED);
  paint.DrawStringAt(0, 0, rate, &Font12, COLORED);
  epd.SetFrameMemory(paint.GetImage(), 155, 120, paint.GetWidth(), paint.GetHeight());
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