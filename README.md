# wifi-esp32-usb
use webdev copy files to sd card and tinyusb change esp32 to usb memory disk.
esp32s2 to sd card module connect pin:
#define SD_MISO  37
#define SD_MOSI  39
#define SD_SCK   38
#define SD_CS    40

place wifi.txt to the sd card:
wifi.txt:
ssid1,pass1
ssid2,pass2
.
.
.

visite http://your_esp32s2_ip with file explorer.
