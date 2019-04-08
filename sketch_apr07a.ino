
#include <WiFi.h>
#include <HTTPClient.h>
#include "secrets.h"  // Includes SSID, Wifi & Webhook URLs

#define DEBUG 1 // Toggles serial print code
#define WIFI_TIMEOUT 15 // wifi timeout in seconds

const int buttonDelay = 2000;

struct Button {
  const uint8_t PIN;
  uint32_t numberKeyPresses;
  bool pressed;
  const char* URL;
  const char* colour;
  long lastPressed;
};

Button greenButton = {23, 0, false, greenButtonURL,"green",0}; // Green button connected to pin 23
Button yellowButton = {18, 0, false, yellowButtonURL, "yellow",0}; // Yellow button connected to pin 18
Button redButton = {14, 0, false, redButtonURL, "red",0}; // Red button connected to pin 14


// callbacks for interupts

void IRAM_ATTR greenButtonPressed() {

 unsigned long interrupt_time = millis();
 
 // If interrupts come faster than buttonDelay, assume it's a bounce and ignore
 if (interrupt_time - greenButton.lastPressed > buttonDelay && greenButton.pressed != true) 
 {

   greenButton.numberKeyPresses += 1;
   greenButton.pressed = true;
 
 }
 greenButton.lastPressed = interrupt_time;
 greenButton.pressed = false;

 
}

void IRAM_ATTR yellowButtonPressed() {



 unsigned long interrupt_time = millis();
 
 // If interrupts come faster than buttonDelay, assume it's a bounce and ignore
 if (interrupt_time - yellowButton.lastPressed > buttonDelay) 
 {

  yellowButton.numberKeyPresses += 1;
  yellowButton.pressed = true;
 
 }
 yellowButton.lastPressed = interrupt_time;
 yellowButton.pressed = false;

}

void IRAM_ATTR redButtonPressed() {


 unsigned long interrupt_time = millis();
 
 // If interrupts come faster than buttonDelay, assume it's a bounce and ignore
 if (interrupt_time - redButton.lastPressed > buttonDelay) 
 {

  redButton.numberKeyPresses += 1;
  redButton.pressed = true;
 
 }
 redButton.lastPressed = interrupt_time;
 redButton.pressed = false;
}



void setup() {
  
  Serial.begin(9600);

   // Set pin inputs with built-in pullup. Simplifies physical circuit.
  
  pinMode(greenButton.PIN, INPUT_PULLUP);          
  pinMode(yellowButton.PIN, INPUT_PULLUP);          
  pinMode(redButton.PIN, INPUT_PULLUP);           

  // Use interrupts to catch button pushes. This helps avoid missing a button push whilst calling a webhook/otherwise busy
  
  attachInterrupt(greenButton.PIN, greenButtonPressed, FALLING);  
  attachInterrupt(yellowButton.PIN, yellowButtonPressed, FALLING); 
  attachInterrupt(redButton.PIN, redButtonPressed, FALLING); 
  
  initWifi(ssid, password, WIFI_TIMEOUT);
  

}



void initWifi(const char* network, const char* pass, int timeout) {

  Serial.print("Connecting to: "); 
  Serial.print(network);

  WiFi.begin(network, pass);  
  
  while(WiFi.status() != WL_CONNECTED  && (timeout-- > 0)) {
    delay(1000);
    Serial.print(".");
  }
  
  Serial.println("");
  
  if(WiFi.status() != WL_CONNECTED) 
  {
    Serial.println("Failed to connect, going to reboot and try again");
    delay(2000);
    ESP.restart();
  }
  else
  {
    Serial.print("WiFi connected in: "); 
    Serial.print(millis());
    Serial.print(", IP address: "); 
    Serial.println(WiFi.localIP());
  }
  
}

bool callWebHook(const char* URL, const char* colour) {
  
  if ((WiFi.status() == WL_CONNECTED)) 
  { 
       
      HTTPClient webHook;
   
      webHook.begin(URL); 
      webHook.addHeader("Content-Type", "application/x-www-form-urlencoded");

      String postContentString = "title:";
      postContentString.concat(colour);
      char* postContent = string2char(postContentString);
      
      Serial.println(postContent);
      int httpCode = webHook.POST(colour);
      webHook.writeToStream(&Serial);
                                                  

      if (httpCode > 0) //Check for the returning code
        { 
          String payload = webHook.getString();        
          Serial.println(httpCode);
          #ifdef DEBUG
            Serial.println(payload);
          #endif
          webHook.end(); //Free the resources
          return true;  // Return true if successful (OK, not really checking success here, just not total failure through lack of connection etc. Should refactor to include checking http success)

        }
   
      else {
          Serial.println("Error on HTTP request");
          webHook.end(); //Free the resources
          return false; // Return false if total failure
      }
   
      
  }

}

void onPressed(Button &pressedButton){ // TODO: onPressed is the wrong name here, need to think of a better one for readability

  if(pressedButton.numberKeyPresses > 0)
  {

    #ifdef DEBUG
      Serial.printf("%s Button has been pressed %u times since last webhook sync\n", pressedButton.colour, pressedButton.numberKeyPresses);
    #endif

    while(pressedButton.numberKeyPresses != 0) //call webhook for each time button pressed since last call
    {
      if(callWebHook(pressedButton.URL, pressedButton.colour)) // only decrement the number of key presses if the webhook call was succesful
      {
        pressedButton.numberKeyPresses--;
      }
      
    }
    
    
  }
  
  
}

char* string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}


void loop() {

  // Work through buttons & check if they've been pressed. 

  onPressed(greenButton);
  onPressed(yellowButton);
  onPressed(redButton);

  // Check if we've been disconnected from WiFi and if so, reconnect

  if(WiFi.status() != WL_CONNECTED)
  {
    Serial.println("Got disconnected from Wifi network, reconnecting...");
    initWifi(ssid, password, WIFI_TIMEOUT);
  }

}
