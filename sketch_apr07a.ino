
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"  // Includes SSID, Wifi & Webhook URLs

#define DEBUG 1 // Toggles serial print code

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
  const char* URL;
};

Button greenButton = {23, 0, false, greenButtonURL}; // Green button connected to pin 23
Button yellowButton = {14, 0, false, yellowButtonURL}; // Yellow button connected to pin 14
Button redButton = {18, 0, false, redButtonURL}; // Red button connected to pin 18


void IRAM_ATTR greenButtonPressed() {
  greenButton.numberKeyPresses += 1;
  greenButton.pressed = true;
}

void IRAM_ATTR yellowButtonPressed() {
  yellowButton.numberKeyPresses += 1;
  yellowButton.pressed = true;
}

void IRAM_ATTR redButtonPressed() {
  redButton.numberKeyPresses += 1;
  redButton.pressed = true;
}



void setup() {
  // put your setup code here, to run once:

Serial.begin(9600);

pinMode(greenButton.PIN, INPUT_PULLUP);           // set pin to input with built-in pullup
pinMode(yellowButton.PIN, INPUT_PULLUP);          // set pin to input with built-in pullup
pinMode(redButton.PIN, INPUT_PULLUP);           // set pin to input with built-in pullup

attachInterrupt(greenButton.PIN, greenButtonPressed, FALLING);  // attach interrupt to pin
attachInterrupt(yellowButton.PIN, yellowButtonPressed, FALLING); // attach interrupt to pin
attachInterrupt(redButton.PIN, redButtonPressed, FALLING); // attach interrupt to pin

initWifi();

}



void initWifi() {

  Serial.print("Connecting to: "); 
  Serial.print(ssid);

  WiFi.begin(ssid, password);  

  int timeout = 10 * 4; // 10 seconds
  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(250);
    Serial.print(".");
  }
    Serial.println("");
  
  if(WiFi.status() != WL_CONNECTED) {
     Serial.println("Failed to connect, going back to sleep");
  }

  Serial.print("WiFi connected in: "); 
  Serial.print(millis());
  Serial.print(", IP address: "); 
  Serial.println(WiFi.localIP());
}

void callWebHook(const char* URL) {

if ((WiFi.status() == WL_CONNECTED)) { 
       
          HTTPClient http;
       
          http.begin(URL); //Specify the URL and certificate
          int httpCode = http.GET();                                                  //Make the request
          if (httpCode > 0) { //Check for the returning code
              String payload = http.getString();
              
              
              Serial.println(httpCode);
              #ifdef DEBUG
                Serial.println(payload);
              #endif

            }
       
          else {
              Serial.println("Error on HTTP request");
          }
       
          http.end(); //Free the resources
  }


  
}





void loop() {


  if (greenButton.pressed) {
        #ifdef DEBUG
          Serial.printf("Green Button has been pressed %u times\n", greenButton.numberKeyPresses);
        #endif
        callWebHook(greenButton.URL);
        greenButton.pressed = false;
    }
  
  if (yellowButton.pressed) {
        #ifdef DEBUG
          Serial.printf("Yellow Button has been pressed %u times\n", yellowButton.numberKeyPresses);
        #endif
        callWebHook(yellowButton.URL);
        yellowButton.pressed = false;
    }
  
  if (redButton.pressed) {
        #ifdef DEBUG
          Serial.printf("Red Button has been pressed %u times\n", redButton.numberKeyPresses);
        #endif
        callWebHook(redButton.URL);
        redButton.pressed = false;
    }



}
