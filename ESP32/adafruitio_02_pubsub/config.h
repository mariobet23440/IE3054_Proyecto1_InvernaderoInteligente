/************************ Adafruit IO Config *******************************/

// visit io.adafruit.com if you need to create an account,
// or if you need your Adafruit IO key.
#define IO_USERNAME  "mariobet04"
#define IO_KEY       "aio_YVYz15lxhT1WHuuHS6HZj27MWr5T"

/******************************* WIFI **************************************/

#define WIFI_SSID "MarioGalaxyA22"
#define WIFI_PASS "contraseña"

// comment out the following lines if you are using fona or ethernet
#include "AdafruitIO_WiFi.h"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
