//   #################################################################
//
//   LED Ear Wings
//
//   Author:         Ray Perfetti
//   Updated:        Feb 03, 2020
//   Version:        v1.0
//   Build guide:    http://
//   Microchip:      ESP8265 or ESP8266 (tested with Wemos D1 Mini)
//
//   Description:    Drives WS2812b LEDs strips for Ear Wings wearable
//
//                   Control via Wifi
//                   - Broadcasts unique SSID
//                   - Hosts a WiFi access point
//                   - Emulates a captive portal to trigger OS behavior to load login page in webview
//                   - Serves remote control HTML webpage
//                   - Hosts API server to control brightness and pattern
//
//   Required library: FastLED
//   Required board:   ESP 8266
//   Use Arduino IDE 1.0 or later
//
//   Code adapted from
//   XY mapping code by Mark Kriegsman
//   RGBShades by macetech
//
//   #################################################################


// Data output to LEDs is on pin D3
#define LED_PIN  D3

// WS2812b color order (Green/Red/Blue)
#define COLOR_ORDER GRB
#define CHIPSET     WS2812B

// Global maximum brightness value, maximum 255
#define MAXBRIGHTNESS 75
#define STARTBRIGHTNESS 50

// Cycle time (milliseconds between pattern changes)
#define cycleTime 15000

// Hue time (milliseconds between hue increments)
#define hueTime 30

// Include FastLED library and other useful files
#include <FastLED.h>
#include "XYmap.h"
#include "utils.h"
#include "effects.h"

//################### SERVER #############
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>

#ifndef APSSID
#define APSSID "LED-Ear-Wings"
// Uncomment the line below to password protect the Wifi controller (1 of 3)
//#define APPSK  "set-password-here"
#endif
const char *ssid = APSSID;
// Uncomment the line below to password protect the Wifi controller (2 of 3)
//const char *password = APPSK;


//Sets up DNS to redirect all traffic to the internal web server.
//This makes the OS think you are being blocked from the internet.
//The OS reacts by loading a webview to allow you to login or agree
//to terms to get online. We use this to serve the homepage which
//acts as the controller. This has the added benefit of disconnecting
//the client when they close the webview or lock their phone helping
//us save power and avoid persistent connections.

const byte DNS_PORT = 53;
IPAddress apIP(172, 217, 28, 1);
DNSServer dnsServer;

ESP8266WebServer server(80);

// list of functions that will be displayed
functionList effectList[] = {
  threeSine,      //Fast color wash in rows ***
  plasma,         //Color spiral outward ****
  confetti,       //Sparkles changing colors **
  rider,          //Pulse rows in and out **
  slantBars,      //Non-dense smooth color spiral ***
  colorFill,      //Color fan stacato **
  sideRain          //Sparkles outward, non-dense **
};

const byte numEffects = (sizeof(effectList) / sizeof(effectList[0]));

// Runs one time at the start of the program (power up or reset)
void setup() {

  if (currentEffect > (numEffects - 1)) currentEffect = 0;

  // write FastLED configuration data
  FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, LAST_VISIBLE_LED + 1);

  // set global brightness value
  FastLED.setBrightness( scale8(currentBrightness, MAXBRIGHTNESS) );


  // FOR WEB SERVER
  delay(1000);
  //Serial.begin(115200);
  //Serial.println();
  //Serial.print("Configuring access point...");

  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);

  // Uncomment the line below to password protect the Wifi controller (3 of 3)
  //WiFi.softAP(ssid, password);

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);


  IPAddress myIP = WiFi.softAPIP();
  //Serial.print("AP IP address: ");
  //Serial.println(myIP);

  // Handle server requests
  server.on("/", handleRoot);
  server.on("/hotspot-detect.html", handleCaptivePortal);
  server.on("/bright1", brightLow);
  server.on("/bright2", brightMed);
  server.on("/bright3", brightOffensive);
  server.on("/scenethreeSine", scenethreeSine);
  server.on("/sceneplasma", sceneplasma);
  server.on("/sceneconfetti", sceneconfetti);
  server.on("/scenerider", scenerider);
  server.on("/sceneslantBars", sceneslantBars);
  server.on("/scenecolorFill", scenecolorFill);
  server.on("/scenesideRain", scenesideRain);
  server.on("/scenecycle", scenecycle);
  server.onNotFound(handleCaptivePortal);
  server.begin();

  //Serial.println("HTTP server started");

}


// Runs over and over until power off or reset
void loop()
{

  dnsServer.processNextRequest();


  currentMillis = millis(); // save the current timer value

  // switch to a new effect every cycleTime milliseconds
  if (currentMillis - cycleMillis > cycleTime && autoCycle == true) {
    cycleMillis = currentMillis;
    if (++currentEffect >= numEffects) currentEffect = 0; // loop to start of effect list
    effectInit = false; // trigger effect initialization when new effect is selected
  }

  // increment the global hue value every hueTime milliseconds
  if (currentMillis - hueMillis > hueTime) {
    hueMillis = currentMillis;
    hueCycle(1); // increment the global hue value
  }

  // run the currently selected effect every effectDelay milliseconds
  if (currentMillis - effectMillis > effectDelay) {
    effectMillis = currentMillis;
    effectList[currentEffect](); // run the selected effect function
    random16_add_entropy(1); // make the random values a bit more random-ish
  }

  // run a fade effect too if the confetti effect is running
  if (effectList[currentEffect] == confetti) fadeAll(1);

  FastLED.show(); // send the contents of the led memory to the LEDs

  //For Web Server
  server.handleClient();

}

//################### WEB SERVER #############

void handleRoot() {
  char temp[1300];

  snprintf(temp, 1300,

           "<html>\
  <head>\
    <meta name=\"viewport\" content=\"width=device-width\"/>\
    <title>LED Ear Wings</title>\
    <style>\
      body { background-color: #141F26; font-family: Arial, Helvetica, Sans-Serif; Color: #F2BC79; }\
      a { color: #D9CDBF; }\
    </style>\
  </head>\
  <body>\
    <h1>#ledEarWings</h1>\
    <h3>by Ray Perfetti - Kostume Kult <br>@raypp2</h3>\
    <p><br>Current brightness: %d <ul> \
    <li><a href=\"./bright1\">Low (25)</a></li>\
    <li><a href=\"./bright2\">Med (50)</a></li>\
    <li><a href=\"./bright3\">Offensive (75)</a></li>\
    </ul></p><br>\
    <p>Now showing pattern: %d <br><ol>\
    <li><a href=\"./scenethreeSine\">Color Burst</a></li>\
    <li><a href=\"./sceneplasma\">Rainbow Swirl</a></li>\
    <li><a href=\"./sceneconfetti\">Confetti</a></li>\
    <li><a href=\"./scenerider\">Radial Breathing</a></li>\
    <li><a href=\"./sceneslantBars\">Tilt a Whirl</a></li>\
    <li><a href=\"./scenecolorFill\">Color Wipe</a></li>\
    <li><a href=\"./scenesideRain\">Star Field</a></li>\
    <li><a href=\"./scenecycle\">Cycle All</a></li>\
    </ol></p><br>\
    <h2>Learn how to make your own <br><a href=\"#\" style=\"text-decoration: none\">bit.ly/ear-wings </a></h2>\
    <p>A gift for Love Burn 2020.</p>\
    <p>Thanks for inspiration from <br>@wow_elec_tron<br>@macetech_</p>\
  </body>\
</html>",

           currentBrightness, currentEffect + 1
          );
  server.send(200, "text/html", temp);
}

void handleCaptivePortal() {
  char temp[300];

  snprintf(temp, 300,

           "<html>\
  <head>\
    <meta name=\"viewport\" content=\"width=device-width\"/>\
    <meta http-equiv=\"refresh\" content=\"0; url=http://led-ear-wings.com\">\
    <title>LED Ear Wings</title>\
    <style>\
      body { background-color: #141F26; font-family: Arial, Helvetica, Sans-Serif; Color: #F2BC79; }\
      a { color: #D9CDBF; }\
    </style>\
  </head>\
  <body>\
    <h1>LED Ear Wings</h1>\
    <h3>Loading controller...</h3>\
  </body>\
</html>");
  server.send(200, "text/html", temp);
}

void brightLow() {
  currentBrightness = 25;
  FastLED.setBrightness(currentBrightness);
  handleRoot();
}

void brightMed() {
  currentBrightness = 50;
  FastLED.setBrightness(currentBrightness);
  handleRoot();
}

void brightOffensive() {
  currentBrightness = 100;
  FastLED.setBrightness(currentBrightness);
  handleRoot();
}

void scenethreeSine() {
  currentEffect = 0;
  cycleMillis = currentMillis;
  autoCycle = false;
  effectInit = false; // trigger effect initialization when new effect is selected
  handleRoot();
}

void sceneplasma() {
  currentEffect = 1;
  cycleMillis = currentMillis;
  autoCycle = false;
  effectInit = false; // trigger effect initialization when new effect is selected
  handleRoot();
}

void sceneconfetti() {
  currentEffect = 2;
  cycleMillis = currentMillis;
  autoCycle = false;
  effectInit = false; // trigger effect initialization when new effect is selected
  handleRoot();
}

void scenerider() {
  currentEffect = 3;
  cycleMillis = currentMillis;
  autoCycle = false;
  effectInit = false; // trigger effect initialization when new effect is selected
  handleRoot();
}

void sceneslantBars() {
  currentEffect = 4;
  cycleMillis = currentMillis;
  autoCycle = false;
  effectInit = false; // trigger effect initialization when new effect is selected
  handleRoot();
}

void scenecolorFill() {
  currentEffect = 5;
  cycleMillis = currentMillis;
  autoCycle = false;
  effectInit = false; // trigger effect initialization when new effect is selected
  handleRoot();
}

void scenesideRain() {
  currentEffect = 6;
  cycleMillis = currentMillis;
  autoCycle = false;
  effectInit = false; // trigger effect initialization when new effect is selected
  handleRoot();
}

void scenecycle() {
  autoCycle = true; // toggle auto cycle mode

  //Cycling to the next effect provides visual feedback that the action was recognized
  cycleMillis = currentMillis;
  if (++currentEffect >= numEffects) currentEffect = 0; // loop to start of effect list
  effectInit = false; // trigger effect initialization when new effect is selected
    
  handleRoot();
}


void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  Serial.println("Attempted access to ");
  Serial.print(server.uri());

  server.send(404, "text/plain", message);
}
