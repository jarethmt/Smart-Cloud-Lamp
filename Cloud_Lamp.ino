/*
  Rui Santos
  Complete project details at Complete project details at https://RandomNerdTutorials.com/esp8266-nodemcu-http-get-open-weather-map-thingspeak-arduino/

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  Code compatible with ESP8266 Boards Version 3.0.0 or above 
  (see in Tools > Boards > Boards Manager > ESP8266)
*/

#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include <Arduino_JSON.h>
#include <FastLED.h>



const char* ssid = "YOUR WIFI SSID";
const char* password = "YOUR WIFI PASSWORD";

String openWeatherMapApiKey = "XXXXXXXXXXXXXXXXXXXXXXXX";

String city = "Pittsburgh";
String countryCode = "US";

// THE DEFAULT TIMER IS SET TO 10 SECONDS FOR TESTING PURPOSES
// For a final application, check the API call limits per hour/minute to avoid getting blocked/banned
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
//unsigned long timerDelay = 10000;

String jsonBuffer;

String last_status = "\"Clear\"";


#define DATA_PIN            13
#define NUM_LEDS            208
#define LED_TYPE            WS2812B
#define COLOR_ORDER         GRB
#define BRIGHTNESS  150

struct CHSV ledshsv[NUM_LEDS];                                 // Initialize our CHSV reference array.
struct CRGB leds[NUM_LEDS];                                    // This is what we are displaying.
struct CRGB ledscrgb[NUM_LEDS];                                // Initialize our CRGB reference array.



int first_run = 1;
int first_drizzle = 1;
int first_rain = 1;

void setup() {

  delay( 3000); // 3 second delay for boot recovery, and a moment of silence
  Serial.begin(115200);

  //LED initialization
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );
  FastLED.setBrightness(  BRIGHTNESS );

  //Wifi Initialization
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
 
  Serial.println("Timer set to 10 seconds (timerDelay variable), it will take 10 seconds before publishing the first reading.");


  handleOTAUpdate();
}

void loop() {
  // Send an HTTP GET request
  if (first_run || (millis() - lastTime) > timerDelay) {
    
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
      first_run = false;
      updateWeatherStatus();
    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();
  }

  else{
    runLightPattern(last_status);
  }

  ArduinoOTA.handle();
  
}

void handleOTAUpdate(){
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_FS
      type = "filesystem";
    }

    // NOTE: if updating FS this would be the place to unmount FS using FS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  ArduinoOTA.begin();
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    //Serial.print("HTTP Response code: ");
    //Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}



void updateWeatherStatus(){
  String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + city + "," + countryCode + "&APPID=" + openWeatherMapApiKey;
      
  jsonBuffer = httpGETRequest(serverPath.c_str());
  JSONVar myObject = JSON.parse(jsonBuffer);

  // JSON.typeof(jsonVar) can be used to get the type of the var
  if (JSON.typeof(myObject) == "undefined") {
    Serial.println("Parsing input failed!");
    return;
  }


  String cur_status = JSON.stringify(myObject["weather"][0]["main"]);

  Serial.println("Current weather status: " + cur_status);

  runLightPattern(cur_status);

  last_status = cur_status;
}


void runLightPattern(const String cur_status){
  if( String("\"Clear\"") == cur_status ){
    lightsClear();
  }
  else if( String("\"Drizzle\"") == cur_status ||
           String("\"Mist\"") == cur_status ){
    lightsDrizzle();
  }
  else if( String("\"Thunderstorm\"") == cur_status ){
    lightsThunderstorm();
  }
  else if( String("\"Rain\"") == cur_status ){
    lightsRain();
  }
  else if( String("\"Snow\"") == cur_status ){
    lightsSnow();
  }
  else if( String("\"Clouds\"") == cur_status ){
    lightsCloudy();
  }
  else{
    lightsClear();
  }

}


void lightsClear(){
  Serial.println("Clear pattern");
  for( int i = 0; i < NUM_LEDS; ++i) {
      leds[i] = CRGB(170, 160, 25);
      //leds[i] = CRGB::Red;
  }
  delay(1000);
  FastLED.show();
}


void lightsCloudy(){
  Serial.println("Cloudy Pattern");

  int base_r = 70;
  int base_g = 70;
  int base_b = 100;

  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(base_r, base_g, base_b);
  }


  //Night rider style pattern of off LEDs, 7 wide
  for( int i = 0; i < NUM_LEDS; i++) {

    leds[i - 4] = CRGB(base_r, base_g, base_b);

    if(i - 3 > 0){
      leds[i - 3] = CRGB::Black;
    }
    if(i - 2 > 0){
      leds[i - 2] = CRGB::Black;
    }
    if(i - 1 > 0){
      leds[i - 1] = CRGB::Black;
    }
    if(i + 1 < NUM_LEDS){
      leds[i + 1] = CRGB::Black;
    }
    else{
      leds[2] = CRGB::Black;
    }
    if(i + 2 < NUM_LEDS){
      leds[i + 2] = CRGB::Black;
    }
    else{
      leds[1] = CRGB::Black;
    }
    if(i + 3 < NUM_LEDS){
      leds[i + 3] = CRGB::Black;
    }
    else{
      leds[0] = CRGB::Black;
    }
    
    
    leds[i] = CRGB::Black;

    FastLED.show();
    delay(40);
    
  }

}



void lightsDrizzle(){

  Serial.println("Drizzle Pattern");

  int base_r = 50;
  int base_g = 50;
  int base_b = 200;

  //If this is the first drizzle run, light all the lights up to the right color
   if(first_drizzle){
    for( int i = 0; i < NUM_LEDS; i++) {
      leds[i].setRGB(base_r, base_g, base_b);
      leds[i].maximizeBrightness(100);
    }
    FastLED.show();
    first_drizzle = false;
  }

  
  
  //We're going to fade some LED groups in and out
  int rand_index = random(8, NUM_LEDS - 8);
  int rand_index2 = random(8, NUM_LEDS - 8);

  int leds_to_fade[] = {
    rand_index - 6,
    rand_index - 5,
    rand_index - 4,
    rand_index - 3,
    rand_index - 2,
    rand_index - 1,
    rand_index,
    rand_index + 1,
    rand_index + 2,
    rand_index + 3,
    rand_index + 4,
    rand_index + 5,
    rand_index + 6,

    rand_index2 - 6,
    rand_index2 - 5,
    rand_index2 - 4,
    rand_index2 - 3,
    rand_index2 - 2,
    rand_index2 - 1,
    rand_index2,
    rand_index2 + 1,
    rand_index2 + 2,
    rand_index2 + 3,
    rand_index2 + 4,
    rand_index2 + 5,
    rand_index2 + 6,

  };

  fadeDown(leds_to_fade, sizeof(leds_to_fade)/sizeof(leds_to_fade[0]), CRGB(base_r, base_g, base_b), 100, 20);
  fadeUp(leds_to_fade, sizeof(leds_to_fade)/sizeof(leds_to_fade[0]), CRGB(base_r, base_g, base_b), 100, 20);
  

  


}




void lightsRain(){

  Serial.println("Rain Pattern");

  int base_r = 40;
  int base_g = 0;
  int base_b = 150;

  //If this is the first drizzle run, light all the lights up to the right color
   if(first_rain){
    for( int i = 0; i < NUM_LEDS; i++) {
      leds[i].setRGB(base_r, base_g, base_b);
      leds[i].maximizeBrightness(60);
    }
    FastLED.show();
    first_rain = false;
  }

  
  
  //We're going to fade some LED groups in and out
  int rand_index = random(8, NUM_LEDS - 8);
  int rand_index2 = random(8, NUM_LEDS - 8);
  int rand_index3 = random(8, NUM_LEDS - 8);
  int rand_index4 = random(8, NUM_LEDS - 8);
  int rand_index5 = random(8, NUM_LEDS - 8);
  int rand_index6 = random(8, NUM_LEDS - 8);
  int rand_index7 = random(8, NUM_LEDS - 8);

  int leds_to_fade[] = {
    rand_index - 7,
    rand_index - 6,
    rand_index - 5,
    rand_index - 4,
    rand_index - 3,
    rand_index - 2,
    rand_index - 1,
    rand_index,
    rand_index + 1,
    rand_index + 2,
    rand_index + 3,
    rand_index + 4,
    rand_index + 5,
    rand_index + 6,
    rand_index + 7,

    rand_index2 - 7,
    rand_index2 - 6,
    rand_index2 - 5,
    rand_index2 - 4,
    rand_index2 - 3,
    rand_index2 - 2,
    rand_index2 - 1,
    rand_index2,
    rand_index2 + 1,
    rand_index2 + 2,
    rand_index2 + 3,
    rand_index2 + 4,
    rand_index2 + 5,
    rand_index2 + 6,
    rand_index2 + 7,

    rand_index3 - 7,
    rand_index3 - 6,
    rand_index3 - 5,
    rand_index3 - 4,
    rand_index3 - 3,
    rand_index3 - 2,
    rand_index3 - 1,
    rand_index3,
    rand_index3 + 1,
    rand_index3 + 2,
    rand_index3 + 3,
    rand_index3 + 4,
    rand_index3 + 5,
    rand_index3 + 6,
    rand_index3 + 7,

    rand_index4 - 7,
    rand_index4 - 6,
    rand_index4 - 5,
    rand_index4 - 4,
    rand_index4 - 3,
    rand_index4 - 2,
    rand_index4 - 1,
    rand_index4,
    rand_index4 + 1,
    rand_index4 + 2,
    rand_index4 + 3,
    rand_index4 + 4,
    rand_index4 + 5,
    rand_index4 + 6,
    rand_index4 + 7,

    rand_index5 - 7,
    rand_index5 - 6,
    rand_index5 - 5,
    rand_index5 - 4,
    rand_index5 - 3,
    rand_index5 - 2,
    rand_index5 - 1,
    rand_index5,
    rand_index5 + 1,
    rand_index5 + 2,
    rand_index5 + 3,
    rand_index5 + 4,
    rand_index5 + 5,
    rand_index5 + 6,
    rand_index5 + 7,

    rand_index6 - 7,
    rand_index6 - 6,
    rand_index6 - 5,
    rand_index6 - 4,
    rand_index6 - 3,
    rand_index6 - 2,
    rand_index6 - 1,
    rand_index6,
    rand_index6 + 1,
    rand_index6 + 2,
    rand_index6 + 3,
    rand_index6 + 4,
    rand_index6 + 5,
    rand_index6 + 6,
    rand_index6 + 7,

    rand_index7 - 7,
    rand_index7 - 6,
    rand_index7 - 5,
    rand_index7 - 4,
    rand_index7 - 3,
    rand_index7 - 2,
    rand_index7 - 1,
    rand_index7,
    rand_index7 + 1,
    rand_index7 + 2,
    rand_index7 + 3,
    rand_index7 + 4,
    rand_index7 + 5,
    rand_index7 + 6,
    rand_index7 + 7
  };

  fadeDown(leds_to_fade, sizeof(leds_to_fade)/sizeof(leds_to_fade[0]), CRGB(base_r, base_g, base_b), 60, 0);
  fadeUp(leds_to_fade, sizeof(leds_to_fade)/sizeof(leds_to_fade[0]), CRGB(base_r, base_g, base_b), 60, 0);
  

}




void lightsSnow(){

  Serial.println("Snow Pattern");

  int base_r = 80;
  int base_g = 80;
  int base_b = 80;

  //If this is the first drizzle run, light all the lights up to the right color

  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i].setRGB(base_r, base_g, base_b);
    leds[i].maximizeBrightness(40);
  }
  FastLED.show();


  
  
  //We're going to fade some LED groups in and out
  int rand_index = random(8, NUM_LEDS - 8);
  int rand_index2 = random(8, NUM_LEDS - 8);
  int rand_index3 = random(8, NUM_LEDS - 8);
  int rand_index4 = random(8, NUM_LEDS - 8);
  int rand_index5 = random(8, NUM_LEDS - 8);
  int rand_index6 = random(8, NUM_LEDS - 8);
  int rand_index7 = random(8, NUM_LEDS - 8);
  int rand_index8 = random(8, NUM_LEDS - 8);
  int rand_index9 = random(8, NUM_LEDS - 8);
  int rand_index10 = random(8, NUM_LEDS - 8);
  int rand_index11 = random(8, NUM_LEDS - 8);
  int rand_index12 = random(8, NUM_LEDS - 8);
  int rand_index13 = random(8, NUM_LEDS - 8);
  int rand_index14 = random(8, NUM_LEDS - 8);
  int rand_index15 = random(8, NUM_LEDS - 8);


  leds[rand_index].maximizeBrightness(190);
  leds[rand_index2].maximizeBrightness(190);
  leds[rand_index3].maximizeBrightness(190);
  leds[rand_index4].maximizeBrightness(190);
  leds[rand_index5].maximizeBrightness(190);
  leds[rand_index6].maximizeBrightness(190);
  leds[rand_index7].maximizeBrightness(190);
  leds[rand_index8].maximizeBrightness(190);
  leds[rand_index9].maximizeBrightness(190);
  leds[rand_index10].maximizeBrightness(190);
  leds[rand_index11].maximizeBrightness(190);
  leds[rand_index12].maximizeBrightness(190);
  leds[rand_index13].maximizeBrightness(190);
  leds[rand_index14].maximizeBrightness(190);
  leds[rand_index15].maximizeBrightness(190);

  FastLED.show();
  delay(300);
  
  

}







void lightsThunderstorm(){
  Serial.println("Thunerdstorm Pattern");

  int base_r = 50;
  int base_g = 0;
  int base_b = 100;

  for( int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB(base_r, base_g, base_b);
  }

  FastLED.show();

  //now let's do the drizzle
  //I'm thinking random brighter hue pulses at random indexes

  int rain_r = 70;
  int rain_g = 10;
  int rain_b = 120;

  for( int i = 0; i < 30; i++) {
    int rand_index = random(5, NUM_LEDS - 5);
    int rand_index_2 = random(5, NUM_LEDS - 5);
    int rand_index_3 = random(5, NUM_LEDS - 5);
    int rand_index_4 = random(5, NUM_LEDS - 5);
    int rand_index_5 = random(5, NUM_LEDS - 5);

    leds[rand_index - 2] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index - 1] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index + 1] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index + 2] = CRGB(rain_r, rain_g, rain_b);

    leds[rand_index_2 - 2] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_2 - 1] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_2] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_2 + 1] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_2 + 2] = CRGB(rain_r, rain_g, rain_b);

    leds[rand_index_3 - 2] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_3 - 1] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_3] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_3 + 1] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_3 + 2] = CRGB(rain_r, rain_g, rain_b);

    leds[rand_index_4 - 2] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_4 - 1] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_4] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_4 + 1] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_4 + 2] = CRGB(rain_r, rain_g, rain_b);

    leds[rand_index_5 - 2] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_5 - 1] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_5] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_5 + 1] = CRGB(rain_r, rain_g, rain_b);
    leds[rand_index_5 + 2] = CRGB(rain_r, rain_g, rain_b);


    FastLED.show();
    delay(50);

    leds[rand_index - 2] = CRGB(base_r, base_g, base_b);
    leds[rand_index - 1] = CRGB(base_r, base_g, base_b);
    leds[rand_index] = CRGB(base_r, base_g, base_b);
    leds[rand_index + 1] = CRGB(base_r, base_g, base_b);
    leds[rand_index + 2] = CRGB(base_r, base_g, base_b);

    leds[rand_index_2 - 2] = CRGB(base_r, base_g, base_b);
    leds[rand_index_2 - 1] = CRGB(base_r, base_g, base_b);
    leds[rand_index_2] = CRGB(base_r, base_g, base_b);
    leds[rand_index_2 + 1] = CRGB(base_r, base_g, base_b);
    leds[rand_index_2 + 2] = CRGB(base_r, base_g, base_b);

    leds[rand_index_3 - 2] = CRGB(base_r, base_g, base_b);
    leds[rand_index_3 - 1] = CRGB(base_r, base_g, base_b);
    leds[rand_index_3] = CRGB(base_r, base_g, base_b);
    leds[rand_index_3 + 1] = CRGB(base_r, base_g, base_b);  
    leds[rand_index_3 + 2] = CRGB(base_r, base_g, base_b);

    leds[rand_index_4 - 2] = CRGB(base_r, base_g, base_b);
    leds[rand_index_4 - 1] = CRGB(base_r, base_g, base_b);
    leds[rand_index_4] = CRGB(base_r, base_g, base_b);
    leds[rand_index_4 + 1] = CRGB(base_r, base_g, base_b);
    leds[rand_index_4 + 2] = CRGB(base_r, base_g, base_b);

    leds[rand_index_5 - 2] = CRGB(base_r, base_g, base_b);
    leds[rand_index_5 - 1] = CRGB(base_r, base_g, base_b);
    leds[rand_index_5] = CRGB(base_r, base_g, base_b);
    leds[rand_index_5 + 1] = CRGB(base_r, base_g, base_b);
    leds[rand_index_5 + 2] = CRGB(base_r, base_g, base_b);

    FastLED.show();
  }
}



int brightness = 240;

int FastLED_fade_counter = 80;


void  fadeUp (int* lightIndex, int numLights, CRGB color, int maxBrightness, int minBrightness){
  FastLED_fade_counter = minBrightness;
  for(int i = minBrightness; i < maxBrightness; i++){
      for( unsigned int x = 0; x < numLights; x++ ){
        leds[lightIndex[x]] = color;
        leds[lightIndex[x]].maximizeBrightness(FastLED_fade_counter);  // 'FastLED_fade_counter' How high we want to fade up to 255 = maximum.
      }
      FastLED.show();
      FastLED_fade_counter ++ ;   // Increment
      delay(1);
  }
}


void fadeDown (int* lightIndex, int numLights, CRGB color, int maxBrightness, int minBrightness){
  FastLED_fade_counter = maxBrightness;
  for(int i = minBrightness; i < maxBrightness; i++){
      for( unsigned int x = 0; x < numLights; x++ ){
        leds[lightIndex[x]] = color;
        leds[lightIndex[x]].maximizeBrightness(FastLED_fade_counter);  // 'FastLED_fade_counter' How high we want to fade up to 255 = maximum.
      }
      FastLED.show();
      FastLED_fade_counter -- ;   // Decrement
      delay(1);
  }
}

