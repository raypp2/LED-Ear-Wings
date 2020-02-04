LED Ear Wings
===========

Code for LED Wearable project described here:
http://biy.ly/ledEarWings

Drives WS2812b LEDs strips for Ear Wings wearable

Control via Wifi
* Broadcasts unique SSID
* Hosts a WiFi access point
* Emulates a captive portal to trigger OS behavior to load login page in webview
* Serves remote control HTML webpage
* Hosts API server to control brightness and pattern

Version:        v1.0
Microchip:      ESP8265 or ESP8266 (tested with Wemos D1 Mini)
Required library: FastLED
Use Arduino IDE 1.6.5 or later

## Code adapted from
XY mapping code by Mark Kriegsman
https://macetech.github.io/FastLED-XY-Map-Generator/

RGBShades by macetech
http://docs.macetech.com/doku.php/rgb_shades#install_fastled_library