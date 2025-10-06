#include <Arduino.h>
#include <SoftwareSerial.h>

#define LED1 10

// == Serial BT ==

const byte rxPin = 2; // RX pin of the SoftwareSerial module
const byte txPin = 3; // TX pin of the SoftwareSerial module

SoftwareSerial BT(rxPin, txPin); // create a SoftwareSerial object

// == Variables ==

char RX;

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    BT.begin(115200);
    pinMode(LED1, OUTPUT); // set the LED pin as an output
}

void loop()
{
    // put your main code here, to run repeatedly:
    if (BT.available())
    {
        RX = BT.read();
        Serial.write(BT.read());
        if (RX == 'L')
        {
            BT.println("LED1 1 Seg Pulse");
            digitalWrite(LED1, HIGH);
            delay(1000);
            digitalWrite(LED1, LOW);
        }
    }
}