#include <Arduino.h>

#define SAMPLES 100

float voltage = 0;
float temp = 0;

int ADC0 = A5;
float adc_samples = 0;
int adc_counter = 0;

void sample_ADC();

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
}

void loop()
{
    // put your main code here, to run repeatedly:
    sample_ADC();
    delay(1000);
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
            voltage = (adc_samples / SAMPLES) * (5.0 / 1023.0);

            temp = voltage * 100;
            // temp = int(trunc((((((adc_samples / SAMPLES) * 5.0) / 1023.0)) - 0.12) * 100)); // 0.12 V offset of LM358

            Serial.print("Voltage = ");
            Serial.print(voltage);
            Serial.println(" V");

            Serial.print("Temperature = ");
            Serial.print(temp);
            Serial.println(" C");
        }
    }

    adc_counter = 0;
    adc_samples = 0;

    return;
}