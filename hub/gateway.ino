// Example arduino sketch for using an arduino as a serial gateway.
// This gateway allows using any radio supported by the RadioHead library
// with the Node.js module 'radiohead-serial'.
//
// This example is taken from the original RadioHead examples.
// see http://www.airspayce.com/mikem/arduino/RadioHead/examples.html
//
// This sketch can be used as a gateway between 2 RadioHead radio networks (connected by a serial line),
// or between
// 1 RadioHead radio network and a Unix host.
// It relays all messages received on the radio driver to the serial port
// (using the RH_Serial driver and protocol). And it relays all messages received on the RH_Serial
// driver port to the radio driver.
// Both drivers operate in promiscuous mode and preserve all headers, so this sketch acts as
// a transparent gateway between RH_Serial and and other RadioHead driver.
//
// By replacing RH_Serial with another RadioHead driver, this can act as a universal gateway
// between any 2 RadioHead networks.
//
#include <SPI.h>
#include <RH_ASK.h>
#include <RH_Serial.h>
#include <RHReliableDatagram.h>

// Singleton instance of the radio driver
// You can use other radio drivers if you want

// tx/rx configuration
const int txSpeed = 2000;
const int rxPin = 11;
const int txPin = 12;
const int pttPin = 7;

RH_ASK radio(txSpeed, rxPin, txPin, pttPin);

// Singleton instance of the serial driver which relays all messages
// via Serial to another RadioHead RH_Serial driver, perhaps on a Unix host.
// You could use a different Serial if the arduino has more than 1, eg Serial1 on a Mega
RH_Serial serial(Serial);

void setup()
{
  Serial.begin(9600);
  if (!radio.init()) {
    Serial.println(F("RX initialisation failed"));
    return;
  }

  radio.setPromiscuous(true);

  if (!serial.init()) {
    Serial.println(F("Serial initialisation failed"));
    return;
  }

  serial.setPromiscuous(true);
}

uint8_t buf[RH_SERIAL_MAX_MESSAGE_LEN];

void loop() {
  uint8_t len = sizeof(buf);
  if (radio.recv(buf, &len)) {
    serial.setHeaderTo(radio.headerTo());
    serial.setHeaderFrom(radio.headerFrom());
    serial.setHeaderId(radio.headerId());
    serial.setHeaderFlags(radio.headerFlags(), 0xFF); // Must clear all flags
    serial.send(buf, len);
  }
}
