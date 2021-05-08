#include <Arduino.h>
#include <HTS.h>
#include "BARO.h"
#include "Arduino_APDS9960.h"
#include "PDM.h"

void onPDMdata();

char uartBuf[50];
short sampleBuffer[256];
int samplesRead;

void setup() {
    Serial1.begin(115200);
    HTS.begin();
    BARO.begin();
    APDS.begin();
    PDM.onReceive(onPDMdata);
    PDM.begin(1, 16000);
    pinMode(LED_BUILTIN, OUTPUT);
    Serial1.print("CLR(0);SBC(0);\r\n");
}

void loop() {
    auto temperature = HTS.readTemperature();

    auto humidity = HTS.readHumidity();

    auto pressure = BARO.readPressure();

    while (!APDS.colorAvailable()) {
        delay(5);
    }
    int r, g, b;
    APDS.readColor(r, g, b);

    double l;
    if (samplesRead) {
        long long pow_sum = 0;
        for (int i = 0; i < samplesRead; i++) {
            pow_sum += sampleBuffer[i] * sampleBuffer[i];
        }
        pow_sum /= samplesRead;
        auto p_rms = sqrt(pow_sum);
//        l = 20 * log10(p_rms);
        l = 20 * log10(p_rms / 32768.0) + 122.5;
        Serial.println(l);
        samplesRead = 0;
    }


    delay(100);

    Serial1.print("DCV16(10,10,'");
    sprintf(uartBuf, "Tempe:%.1f C", temperature);
    Serial1.print(uartBuf);
    Serial1.print("',1);\r\n");

    delay(100);

    Serial1.print("DCV16(10,30,'");
    sprintf(uartBuf, "Humi:%.0f %%", humidity);
    Serial1.print(uartBuf);
    Serial1.print("',1);\r\n");

    delay(100);

    Serial1.print("DCV16(10,50,'");
    sprintf(uartBuf, "Pres:%.1f kPa", pressure);
    Serial1.print(uartBuf);
    Serial1.print("',1);\r\n");

    delay(100);

    Serial1.print("DCV16(10,70,'");
    Serial1.print(((r < 30) && (g < 30) && (b < 30)) ?
                  "time:night" : "time:day  ");
    Serial1.print("',1);\r\n");

    delay(100);

    Serial1.print("DCV16(10,90,'");
    sprintf(uartBuf, "Sound:%.0f dB", l);
    Serial1.print(uartBuf);
    Serial1.print("',1);\r\n");

    Serial.print("Temperature = ");
    Serial.print(temperature);
    Serial.println(" °C");
    Serial.print("Humidity    = ");
    Serial.print(humidity);
    Serial.println(" %");

    Serial.print("Pressure = ");
    Serial.print(pressure);
    Serial.println(" kPa");


    if ((r < 30) && (g < 30) && (b < 30)) {
        Serial.println("night");
    } else {
        Serial.println("day");
    }

//    digitalWrite(LED_BUILTIN, HIGH);
//    delay(500);
//    digitalWrite(LED_BUILTIN, LOW);
}

void onPDMdata() {
    // query the number of bytes available
    int bytesAvailable = PDM.available();

    // read into the sample buffer
    PDM.read(sampleBuffer, bytesAvailable);

    // 16-bit, 2 bytes per sample
    samplesRead = bytesAvailable / 2;
}

