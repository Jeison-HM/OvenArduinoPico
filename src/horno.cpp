#include <Arduino.h>
#include <SoftwareSerial.h>

#define HORNO 7
#define LED_STOP 8
#define LED_START 9
#define LED_HIGH 10
#define LED_SET 11
#define LED_LOW 12

#define SAMPLES 100
#define INTERVAL 10
#define UPPER tmpSet + INTERVAL
#define LOWER tmpSet - INTERVAL

#define LOW_STATE 0
#define SET_STATE 1
#define HIGH_STATE 2

// == Serial BT ==

const byte rxPin = 2;
const byte txPin = 3;

// create a SoftwareSerial object
SoftwareSerial BT(rxPin, txPin);

// == Horno ==

const byte hornoPin = 7;

const byte ledLow = 8;
const byte ledSet = 9;
const byte ledHigh = 10;

// == Global Variables ==

char RX;

bool start = false;
bool stop = true;

int tmp = 20;
int tmpSet = 25;

int time = 0;
int timeSet = 60;
bool start_timer = false;

float adc = 0;

// == State Machine ==

int state = LOW_STATE;

// put function declarations here:

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    BT.begin(115200);
    // horno
    pinMode(HORNO, OUTPUT);
    pinMode(LED_STOP, OUTPUT);
    pinMode(LED_START, OUTPUT);
    pinMode(LED_HIGH, OUTPUT);
    pinMode(LED_SET, OUTPUT);
    pinMode(LED_LOW, OUTPUT);
}

void loop()
{
    // put your main code here, to run repeatedly:
    if (stop)
    {
        // == Reset State ==

        time = timeSet;
        start_timer = false;

        digitalWrite(LED_START, LOW);
        digitalWrite(LED_LOW, LOW);
        digitalWrite(LED_SET, LOW);
        digitalWrite(LED_HIGH, LOW);
        digitalWrite(HORNO, LOW);

        // == Stop Blink ==

        digitalWrite(LED_STOP, HIGH);
        delay(250);
        digitalWrite(LED_STOP, LOW);
    }
    else
    {
        // Stop LED Off
        digitalWrite(LED_STOP, LOW);

        if (BT.available())
        {
            RX = BT.read();
            Serial.write(BT.read()); // print
            switch (RX)
            {
            case 'P':
                BT.println("Start Sent");
                start = !digitalRead(start);
                break;
            case 'S':
                BT.println("Stop Sent");
                stop = !digitalRead(stop);
                break;
            }
        }
        // == State Machine ==
        if (start)
        {

            // ADD ADC READ WITH SAMPLES

            switch (state)
            {
            case LOW_STATE:
                if (tmp >= LOWER)
                    state = SET_STATE;
                break;
            case SET_STATE:
                start_timer = true;
                if (tmp < LOWER)
                    state = LOW_STATE;
                else if (tmp > UPPER)
                    state = HIGH_STATE;
                break;
            case HIGH_STATE:
                if (tmp <= UPPER)
                    state = SET_STATE;
                break;
            }
            // output logic
            digitalWrite(LED_START, !stop ? HIGH : LOW);
            digitalWrite(LED_LOW, (tmp <= LOWER && state == LOW_STATE) ? HIGH : LOW);
            digitalWrite(LED_SET, (!(tmp > UPPER) && !(tmp < LOWER) && state == SET_STATE) ? HIGH : LOW);
            digitalWrite(LED_HIGH, (tmp >= UPPER && state == HIGH_STATE) ? HIGH : LOW);
            digitalWrite(HORNO, (tmp <= UPPER && state == SET_STATE) ? HIGH : LOW);
            // timer
            if (start_timer)
            {
                time = time - 1;
                delay(1000);
            }
        }
    }
}

// put function definitions here: