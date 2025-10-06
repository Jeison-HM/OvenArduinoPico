#include <Arduino.h>
#include <SoftwareSerial.h>

const byte rxPin = 2; // RX pin of the SoftwareSerial module
const byte txPin = 3; // TX pin of the SoftwareSerial module

// AT+NAME=GRUPO4
// AT+PSWD="4444"
// AT+ROLE=0
// AT+UART=115200,0,0

SoftwareSerial BT(rxPin, txPin); // create a SoftwareSerial object

void setup()
{
  // put your setup code here, to run once:
  Serial.begin(38400);
  BT.begin(38400);
}

void loop()
{
  // put your main code here, to run repeatedly:
  if (BT.available())
  {
    Serial.write(BT.read());
  }

  if (Serial.available())
  {
    BT.write(Serial.read());
  }
}