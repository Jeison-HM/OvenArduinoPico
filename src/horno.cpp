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

bool start = false;
bool stop = true;

int tmp = 51;
int tmpSet = 40;

int time = 0;
int timeSet = 10;
bool start_timer = false;
int timer_counter = 0;

int ADC0 = A5;
float adc_samples = 0;
int adc_counter = 0;

// put function declarations here:
void serialRead();

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

    // == Serial ==

    serialRead();

    // == Main System ==

    if (stop || !start)
    {
        // == Reset ==

        time = timeSet;
        start_timer = false;
        adc_counter = 0;
        adc_samples = 0;
        timer_counter = 0;

        digitalWrite(LED_START, LOW);
        digitalWrite(LED_LOW, LOW);
        digitalWrite(LED_SET, LOW);
        digitalWrite(LED_HIGH, LOW);
        digitalWrite(HORNO, LOW);

        // == Stop Blink ==

        digitalWrite(LED_STOP, LOW);
        delay(250);
        digitalWrite(LED_STOP, HIGH);
        delay(250);
    }
    else if (start)
    {
        // LEDs
        digitalWrite(LED_STOP, LOW);
        digitalWrite(LED_START, HIGH);

        // Sample ADC
        adc_samples = adc_samples + analogRead(ADC0);
        adc_counter++;

        if (adc_counter == SAMPLES)
        {
            adc_counter = 0;

            tmp = int(trunc((((((adc_samples / SAMPLES) * 5.0) / 1023.0)) - 0.12) * 100)); // 0.12 V offset of LM358
            BT.print("Temperature = ");
            BT.println(tmp);

            adc_samples = 0;

            if (tmp < LOWER)
            {
                digitalWrite(LED_LOW, HIGH);
                digitalWrite(LED_SET, LOW);
                digitalWrite(LED_HIGH, LOW);
                digitalWrite(HORNO, HIGH);
            }
            else if (tmp >= LOWER || tmp <= UPPER)
            {
                start_timer = true;
                digitalWrite(LED_LOW, LOW);
                digitalWrite(LED_SET, HIGH);
                digitalWrite(LED_HIGH, LOW);
                digitalWrite(HORNO, HIGH);
            }
            if (tmp > UPPER)
            {
                start_timer = true;
                digitalWrite(LED_LOW, LOW);
                digitalWrite(LED_SET, LOW);
                digitalWrite(LED_HIGH, HIGH);
                digitalWrite(HORNO, LOW);
            }

            // == Timer ==

            if (start_timer && timer_counter == 4)
            {
                timer_counter = 0;
                time = time - 1;
                BT.print("Time = ");
                BT.println(time);
                if (time == 0)
                {
                    stop = true;
                    start = false;
                    BT.println("TIMEOUT!");
                    BT.println("System Shutdown");
                }
            }

            timer_counter++;
            delay(250);
        }
    }
}

// put function definitions here:
void serialRead()
{
    if (BT.available())
    {
        String input;
        String command;
        String value;

        input = Serial.readStringUntil('\n'); // Read the full line until newline

        input.trim(); // Remove any leading/trailing whitespace

        if (input.length() == 0)
            return;

        int equalIndex = input.indexOf('=');
        if (equalIndex != -1)
        {
            command = input.substring(0, equalIndex);
            value = input.substring(equalIndex + 1);
        }
        else
            command = input;

        // == Terminal ==

        if (command == "START")
        {
            start = !start;
            BT.print("Start : ");
            BT.println(start ? "ON" : "OFF");
        }
        else if (command == "STOP")
        {
            stop = !stop;
            BT.print("Stop : ");
            BT.println(stop ? "ON" : "OFF");
        }
        else if (command == "TMP_SET")
        {
            if (equalIndex != -1)
            {
                tmpSet = value.toInt();
                Serial.print("Temperature set to = ");
                Serial.println(tmpSet);
            }
            else
            {
                Serial.println("Invalid format! Use TMP_SET=XXX");
            }
        }
        else if (command == "TIME_SET")
        {
            if (equalIndex != -1)
            {
                timeSet = value.toInt();
                Serial.print("Time set to = ");
                Serial.println(timeSet);
            }
            else
            {
                Serial.println("Invalid format! Use TIME_SET=XXX");
            }
        }
        else if (command == "SHOW")
        {
            BT.println("--------------------");
            BT.print("Start : ");
            BT.println(start ? "ON" : "OFF");
            BT.print("Stop : ");
            BT.println(stop ? "ON" : "OFF");
            BT.print("Temperature set = ");
            BT.println(tmpSet);
            BT.print("Time set = ");
            BT.println(timeSet);
            BT.println("--------------------");
        }
        else if (command == "HELP")
        {
            BT.println("--------------------");
            Serial.println("SHOW        // Show system state");
            Serial.println("START       // System start (latched)");
            Serial.println("STOP        // System shutdown (latched)");
            Serial.println("TMP_SET     // Temperature set point (TMP_SET=XXX)");
            Serial.println("TIME_SET    // Countdown time to shutdown (TIME_SET=XXX)");
            BT.println("--------------------");
        }
        else
        {
            Serial.println("Command not found! Use HELP to show available commands");
        }
    }
    return;
}