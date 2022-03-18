/**
 * Simple MSC device with SD card
 * author: chegewara
 */
#include "webdavServer.h"
#include "sdusb.h"
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WebServer.h>
#include <time.h>
#include <FFat.h>
#include "webdavServer.h"
#define MAXWIFI 16

struct tm timeinfo;


#define SD_MISO  37
#define SD_MOSI  39
#define SD_SCK   38
#define SD_CS    40
//sd¿¨ SS  MOSI  VDD  CLK  GND  MISO
SDCard2USB dev;
WebdavServerClass wdsserver;


#define NR_OF_LEDS   1
#define NR_OF_ALL_BITS 24*NR_OF_LEDS
rmt_data_t led_data[NR_OF_ALL_BITS];
rmt_obj_t* rmt_send = NULL;
int color[] = { 0x22, 0x11, 0x33 };

void setup()
{
    Serial.begin(115200);
    if ((rmt_send = rmtInit(18, RMT_TX_MODE, RMT_MEM_64)) == NULL)
    {
        Serial.println("init sender failed\n");
    }
    float realTick = rmtSetTick(rmt_send, 100);
    int i = 0;
    for (int col = 0; col < 3; col++) {
        for (int bit = 0; bit < 8; bit++) {
            if (color[col] & (1 << (7 - bit))) {
                led_data[i].level0 = 1;
                led_data[i].duration0 = 8;
                led_data[i].level1 = 0;
                led_data[i].duration1 = 4;

            }
            else {
                led_data[i].level0 = 1;
                led_data[i].duration0 = 4;
                led_data[i].level1 = 0;
                led_data[i].duration1 = 8;
            }
            i++;
        }
    }
    rmtWrite(rmt_send, led_data, NR_OF_ALL_BITS);

    if (dev.initSD(SD_SCK, SD_MISO, SD_MOSI, SD_CS))
    {

        if (dev.begin()) {
            Serial.println("SD card init ok");

            File wifi = SD.open("/wifi.txt", "r");

            
            String wifiName[MAXWIFI];
            String wifiPass[MAXWIFI];
            int wifiCount = 0;
            for (wifiCount = 0; wifiCount < MAXWIFI; ++wifiCount)
            {
                String line = wifi.readStringUntil('\r');
                if (line == NULL || line.isEmpty())  break;
                wifi.read();
                int spIndex = line.indexOf(',');
                wifiName[wifiCount] = line.substring(0, spIndex);
                wifiPass[wifiCount] = line.substring(spIndex+1);
                Serial.printf("wifi: %s, pass: %s \n", wifiName[wifiCount], wifiPass[wifiCount]);
            }

            int connCount = 10;
            WiFi.begin(wifiName[0].c_str(), wifiPass[0].c_str());
            int wifiIndex = 1;
            while (WiFi.status() != WL_CONNECTED) {
                delay(500);
                Serial.print(".");
                if (connCount-- < 0)
                {
                    connCount = 10;
                    Serial.printf("try: %s : %s \n", wifiName[wifiIndex], wifiPass[wifiIndex]);
                    WiFi.begin(wifiName[wifiIndex].c_str(), wifiPass[wifiIndex].c_str());
                    wifiIndex++;
                    if (wifiIndex >= wifiCount) wifiIndex = 0;
                }
            }
            i = 0;
            for (int col = 0; col < 3; col++) {
                for (int bit = 0; bit < 8; bit++) {
                    led_data[i].level0 = 1;
                    led_data[i].duration0 = 4;
                    led_data[i].level1 = 0;
                    led_data[i].duration1 = 8;
                    i++;
                }
            }
            rmtWrite(rmt_send, led_data, NR_OF_ALL_BITS);
            Serial.println("");
            Serial.print("Connected to ");
            Serial.println(wifiName[wifiIndex]);
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());

            if (MDNS.begin("esp32")) {
                Serial.println("MDNS responder started");
            }

            wdsserver.begin();

        }
        else log_e("LUN 1 failed");
    }
    else
    {
        Serial.println("Failed to init SD");

    }

    //test();


}

void loop()
{
    wdsserver.handleClient();
    delay(2);
}



