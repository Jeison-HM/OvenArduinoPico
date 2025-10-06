#include <Arduino.h>
#include <SoftwareSerial.h>
#include <TimerOne.h>

// == Horno ==

#define HORNO_PIN 7

#define LED_STOP 8
#define LED_START 9
#define LED_HIGH 10
#define LED_SET 11
#define LED_LOW 12

#define SAMPLES 100
#define INTERVAL 5
#define UPPER tempSet + INTERVAL
#define LOWER tempSet - INTERVAL

#define BUZZER 13

// == State Machine ==

#define HORNO 0

#define IDLE 0
#define PREHEAT 1
#define HEATING 2

#define CALC 1

#define SEND 0
#define RECEIVE 1

int machine = HORNO;

int state_horno = IDLE;
int state_calc = SEND;

// == Serial BT ==

const byte rxBTPin = 2;
const byte txBTPin = 3;

// create a SoftwareSerial object
SoftwareSerial BT(rxBTPin, txBTPin);

// == Serial Pico ==

const byte rxPicoPin = 4;
const byte txPicoPin = 5;

// create a SoftwareSerial object
SoftwareSerial PICO(rxPicoPin, txPicoPin);

// == Global Variables ==

// horno
bool start = false;
bool stop = true;

int temp = 0;
int tempSet = 0;
int time = 0;
int timeSet = 0;

int ADC0 = A5;
float adc_samples = 0;
int adc_counter = 0;

// calculadora
String calc_message;

// put function declarations here:
void horno();
void calc();
void sample_ADC();
void buzzer_finish();
void buzzer_error();
void reset();
void read_Input();

void setup()
{
    // put your setup code here, to run once:
    BT.begin(115200);
    // horno
    pinMode(HORNO_PIN, OUTPUT);
    pinMode(LED_STOP, OUTPUT);
    pinMode(LED_START, OUTPUT);
    pinMode(LED_HIGH, OUTPUT);
    pinMode(LED_SET, OUTPUT);
    pinMode(LED_LOW, OUTPUT);
    pinMode(BUZZER, OUTPUT);

    digitalWrite(LED_START, HIGH);

    Timer1.initialize(250000); // 250ms
    Timer1.attachInterrupt(read_Input);

    // welcome
    BT.println("Proyecto Horno por grupo 4");
}

void loop()
{
    // put your main code here, to run repeatedly:
    switch (machine)
    {
    case HORNO:
        horno();
        break;
    case CALC:
        calc();
        break;
    default:
        reset();
    }
}

// put function definitions here:
void horno()
{
    if (stop)
        reset();
    else
    {
        digitalWrite(LED_STOP, LOW);

        switch (state_horno)
        {
        case IDLE:
            if (start && !stop && tempSet != 0 && timeSet != 0)
            {
                time = timeSet;
                state_horno = PREHEAT;
            }
            break;
        case PREHEAT:
            sample_ADC();

            if (temp < LOWER)
            {
                digitalWrite(LED_LOW, HIGH);
                digitalWrite(LED_SET, LOW);
                digitalWrite(LED_HIGH, LOW);
            }
            else if (temp >= LOWER || temp <= UPPER)
            {
                state_horno = HEATING;
                digitalWrite(LED_LOW, LOW);
                digitalWrite(LED_SET, HIGH);
                digitalWrite(LED_HIGH, LOW);
            }
            if (temp > UPPER)
            {
                state_horno = HEATING;
                digitalWrite(LED_LOW, LOW);
                digitalWrite(LED_SET, LOW);
                digitalWrite(LED_HIGH, HIGH);
            }

            delay(1000);
            break;
        case HEATING:
            if (temp > UPPER)
            {
                BT.println("Error! Temperature exceeded!");
                BT.println("System shutdown!");
                reset();
                buzzer_error();
            }
            else if (time == 0)
            {
                BT.println("Process finish!");

                state_horno = IDLE;

                start = false;

                temp = 0;
                tempSet = 0;
                time = 0;
                timeSet = 0;

                adc_samples = 0;
                adc_counter = 0;

                digitalWrite(LED_LOW, LOW);
                digitalWrite(LED_SET, LOW);
                digitalWrite(LED_HIGH, LOW);
                digitalWrite(HORNO_PIN, LOW);

                buzzer_finish();
            }
            else
            {
                digitalWrite(HORNO_PIN, HIGH);

                BT.print("Time = ");
                BT.print(time);
                BT.println(" s");

                time = time - 1;

                sample_ADC();

                delay(1000);
            }
            break;
        default:
            reset();
        }
    }
    return;
}

void calc()
{
    switch (state_calc)
    {
    case SEND:
        BT.end();
        PICO.begin(115200);

        PICO.println(calc_message);
        state_calc = RECEIVE;
        break;

    case RECEIVE:
        int count_timeout = 0;

        while (count_timeout < 10000)
        {
            if (PICO.available())
            {
                // Read the full line until newline
                calc_message = PICO.readStringUntil('\n');
                break;
            }
            count_timeout++;
            delay(1);
        }

        PICO.end();
        BT.begin(115200);

        if (count_timeout == 10000)
            BT.println("Error! Timeout exceeded!");
        else
            BT.println(calc_message);

        machine = HORNO;
        state_calc = SEND;

        break;
    default:
        reset();
    }
    return;
}

// read and print ADC temp
void sample_ADC()
{
    while (adc_counter <= SAMPLES)
    {
        adc_samples = adc_samples + analogRead(ADC0);
        adc_counter++;

        if (adc_counter == SAMPLES)
        {
            temp = int(trunc((adc_samples / SAMPLES) * (5.0 / 1023.0) * 100));
            BT.print("Temperature = ");
            BT.print(temp);
            BT.println(" C");
        }
    }

    adc_counter = 0;
    adc_samples = 0;

    return;
}

// 2 short pulses
void buzzer_finish()
{
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);
    delay(50);
    digitalWrite(BUZZER, HIGH);
    delay(100);
    digitalWrite(BUZZER, LOW);
    return;
}

// 3 long pulses
void buzzer_error()
{
    digitalWrite(BUZZER, HIGH);
    delay(1000);
    digitalWrite(BUZZER, LOW);
    delay(100);
    digitalWrite(BUZZER, HIGH);
    delay(1000);
    digitalWrite(BUZZER, LOW);
    delay(100);
    digitalWrite(BUZZER, HIGH);
    delay(1000);
    digitalWrite(BUZZER, LOW);
    return;
}

void reset()
{
    machine = HORNO;

    state_horno = IDLE;
    state_calc = SEND;

    stop = true;
    start = false;

    temp = 0;
    tempSet = 0;
    time = 0;
    timeSet = 0;

    adc_samples = 0;
    adc_counter = 0;

    digitalWrite(LED_LOW, LOW);
    digitalWrite(LED_SET, LOW);
    digitalWrite(LED_HIGH, LOW);
    digitalWrite(HORNO_PIN, LOW);

    // == Stop Blink ==

    digitalWrite(LED_STOP, LOW);
    delay(250);
    digitalWrite(LED_STOP, HIGH);
    delay(250);

    return;
}

void read_Input()
{
    // == Power Blink ==
    digitalWrite(LED_START, !digitalRead(LED_START)); // 250ms

    if (BT.available())
    {
        String input;
        String command;
        String value;

        // Read the full line until newline
        input = BT.readStringUntil('\n');

        // Remove any leading/trailing whitespace
        input.trim();

        if (input.length() == 0)
            return;

        int signIndex = -1;
        for (unsigned int i = 0; i < input.length(); i++)
        {
            if (input[i] == '=' || input[i] == ':')
                signIndex = i;
        }
        if (signIndex == -1)
        {
            command = input;
        }
        else
        {
            command = input.substring(0, signIndex);
            value = input.substring(signIndex + 1);
        }

        // == Terminal ==

        if (command == "STOP")
        {
            stop = !stop;
            BT.print("Stop : ");
            BT.println(stop ? "ON" : "OFF");
        }
        else if (state_horno == IDLE && !stop)
        {
            // only read if in IDLE state
            if (command == "START")
            {
                // check for missing info
                if (tempSet == 0 || timeSet == 0)
                {
                    start = false;
                    BT.println("Error! Desired temp or time not set!");
                    BT.println("See SHOW command to display system status");
                }
                else
                {
                    start = !start;
                    BT.print("Start : ");
                    BT.println(start ? "ON" : "OFF");
                }
            }
            else if (command == "TEMP")
            {
                if (signIndex != -1)
                {
                    tempSet = value.toInt();
                    if (tempSet == 0)
                    {
                        BT.println("Invalid number! Use integer numbers");
                    }
                    else
                    {
                        BT.print("Temperature set to = ");
                        BT.print(tempSet);
                        BT.println(" C");
                    }
                }
                else
                {
                    BT.println("Invalid format! Use TEMP=XX");
                }
            }
            else if (command == "TIME")
            {
                if (signIndex != -1)
                {
                    timeSet = value.toInt();
                    if (timeSet == 0)
                    {
                        BT.println("Invalid number! Use integer numbers");
                    }
                    else
                    {
                        BT.print("Time set to = ");
                        BT.print(timeSet);
                        BT.println(" s");
                    }
                }
                else
                {
                    BT.println("Invalid format! Use TIME=XX");
                }
            }
            else if (command == "Calcular")
            {
                int opIndex = -1;
                for (unsigned int i = 0; i < value.length(); i++)
                {
                    if (value[i] == '+' || value[i] == '-' || value[i] == '*' || value[i] == '/')
                        opIndex = i;
                }
                if (opIndex == -1)
                {
                    BT.println("Invalid operator! Use (+, -, *, /)");
                }
                else
                {
                    int num1 = value.substring(0, opIndex).toInt();
                    int num2 = value.substring(opIndex + 1).toInt();

                    if (num1 == 0 || num2 == 0)
                        BT.println("Invalid number! Use integer numbers");
                    else
                    {
                        machine = CALC;
                        calc_message = value;
                        BT.println("Ok! Calculating...");
                    }
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
                BT.print(tempSet);
                BT.println(" C");
                BT.print("Time set = ");
                BT.print(timeSet);
                BT.println(" s");
                BT.println("--------------------");
            }
            else if (command == "HELP")
            {
                BT.println("--------------------");
                BT.println("SHOW        // Show system state");
                BT.println("START       // System start (latched)");
                BT.println("STOP        // System shutdown (latched)");
                BT.println("TEMP        // Temperature set point (TEMP=XX)");
                BT.println("TIME        // Countdown time to shutdown (TIME=XX)");
                BT.println("                    ");
                BT.println("Calcular    // (+, -, *, /) 2 int numbers (Calcular: Num1 S Num2)");
                BT.println("--------------------");
            }
            else
            {
                BT.println("Command not found! Use HELP to display commands");
            }
        }
    }
    return;
}