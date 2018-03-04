#include <Adafruit_NeoPixel.h>

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#define PIN 14
#define LENGTH_LED 14

ESP8266WebServer server(80);
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LENGTH_LED, PIN);
int gradStep = 0;
bool gradient = false;
uint32_t cFrom; uint32_t cTo;

void handleRoot() {
  String rootMsg = "";
  rootMsg += "<html><head><meta charset='utf-8'></head>";
  rootMsg += "<body><h1 style='text-align: center;'>Lamp of Thomas :</h1>";
  rootMsg += "<fieldset><legend>Colors</legend><label for='colorc' style='margin-right: 100px;'>Center color : </label><input type='color' value='#ffffff' id='colorc'><br>";
  rootMsg += "<label for='colorg' style='margin-right: 100px;'>Left color : </label><input type='color' value='#ffffff' id='colorg'><br>";
  rootMsg += "<label for='colord' style='margin-right: 100px;'>Right color: </label><input type='color' value='#ffffff' id='colord'></fieldset><br>";
  rootMsg += "<fieldset><legend>Colors</legend><label for='cFrom' style='margin-right: 100px;'>From : </label><input type='color' value='#ffffff' id='cFrom'><br>";
  rootMsg += "<label for='cTo' style='margin-right: 100px;'>To : </label><input type='color' value='#ffffff' id='cTo'></fieldset><br>";
  rootMsg += "<input type='button' onclick='sendColor()' value='Change colors'>";
  rootMsg += "<input type'button' onclick='sendGradient()' value='Launch gradient'>";
  rootMsg += "<script type='text/javascript'>function sendColor() {var cc = document.getElementById('colorc').value.slice(1); var cg = document.getElementById('colorg').value.slice(1); var cd = document.getElementById('colord').value.slice(1); window.location.href='http://" + WiFi.localIP().toString() + "/setcolor?cc=' + cc + '&cd=' + cd + '&cg=' + cg;}";
  rootMsg += "function sendGradient() {var cFrom = document.getElementById('cFrom').value.slice(1); var cTo = document.getElementById('cTo').value.slice(1); window.location.href='http://" + WiFi.localIP().toString() + "/setcolor?cFrom=' + cFrom + '&cTo=' + cTo;}</script>";
  rootMsg += "</body></html>";
  
  server.send(200, "text/html", rootMsg);
}

void handleChange() {
  String msg = "";
  if (server.arg("cc") != "" && server.arg("cd") != "" && server.arg("cg") != "") {
    gradient = false;
    
    for (int n = 0; n < 6; n++) {
      uint32_t colorC = strtoul(server.arg("cc").c_str(), NULL, 16);
      strip.setPixelColor(n, colorC);
    }

    for (int n = 6; n < 10; n++) {
      uint32_t colorD = strtoul(server.arg("cd").c_str(), NULL, 16);
      strip.setPixelColor(n, colorD);
    }

    for (int n = 10; n < 14; n++) {
      uint32_t colorG = strtoul(server.arg("cg").c_str(), NULL, 16);
      strip.setPixelColor(n, colorG);
    }
    strip.show();
    msg = "Success ! The colors are changed !";
  }
  else if (server.arg("cFrom") != "" && server.arg("cTo") != "") {    
    cFrom = strtoul(server.arg("cFrom").c_str(), NULL, 16);
    cTo = strtoul(server.arg("cTo").c_str(), NULL, 16);

    if (cFrom - cTo > 0) {
      gradStep = -1;
      gradient = true;
      msg = "Success ! The gradient is changed !";
    }
    else if (cFrom - cTo < 0) {
      gradStep = 1;
      gradient = true;
      msg = "Success ! The gradient is changed !";
    }
    else {
      gradient = false;
      msg = "Fail ! The gradient is not moving...";
    }
  }
  else {
    msg = "PLEASE ! Don't touch the URL <a href=\"" + (String)WiFi.localIP() + "\">Main page</a>";
  }

  server.send (200, "text/html", "<html><head><meta charset=\"utf-8\"></head><body><h1 style=\"text-align: center\">" + msg + "</h1></body></html>");
}

void handleNotFound(){
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void bootSequenceLight() {
  for(int z = 0; z < 3; z++) {
    strip.setPixelColor(0, 255, 0, 0, 127);
    strip.show();
    delay(100);
    strip.setPixelColor(0, 0, 255, 0, 127);
    strip.show();
    delay(100);
    strip.setPixelColor(0, 0, 0, 255, 127);
    strip.show();
    delay(100);
  }
  delay(500);
  strip.setPixelColor(0,0,0,0,0);
  strip.show();
}

void setup(){
  boolean result = WiFi.softAP("MyESP12E", "mamanjesuisla");
  
  server.on("/", handleRoot);

  server.on("/setcolor", handleChange);

  server.onNotFound(handleNotFound);

  server.begin();
  
  strip.begin();
  bootSequenceLight();
}

void loop(){
  server.handleClient();

  if (gradient) {
    uint32_t cTmp = cFrom + gradStep;
    if (cTmp == cTo)
      gradStep *= -1;

    for(int n=0; n < LENGTH_LED; n++) {
      strip.setPixelColor(n, cTmp);
    }
    strip.show();
  }
}
