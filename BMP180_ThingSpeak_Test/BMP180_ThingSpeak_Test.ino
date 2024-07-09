#include <WiFi.h>
#include "ThingSpeak.h"
#include <Adafruit_BMP085.h>
#include <Adafruit_Sensor.h>

const char* ssid = "KarthikT";   // your network SSID (name) 
const char* password = "deathnote";   // your network password

WiFiClient  client;

unsigned long myChannelNumber = 2591801;
const char * myWriteAPIKey = "HSY06NVS56OX9SJI";

// Timer variables
unsigned long lastTime = 0;
unsigned long timerDelay = 30000;

// Variable to hold temperature readings
float temperatureC;
//uncomment if you want to get temperature in Fahrenheit
//float temperatureF;

// Create a sensor object
Adafruit_BMP085 bme; //BME280 connect to ESP32 I2C (GPIO 21 = SDA, GPIO 22 = SCL)

void initBME(){
  if (!bme.begin(0x77)) {
    Serial.println("Could not find a valid BME180 sensor, check wiring!");
    while (1);
  }
}

void setup() {
  Serial.begin(115200);  //Initialize serial
  initBME();
  
  WiFi.mode(WIFI_STA);   
  
  ThingSpeak.begin(client);  // Initialize ThingSpeak
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    
    // Connect or reconnect to WiFi
    if(WiFi.status() != WL_CONNECTED){
      Serial.print("Attempting to connect");
      while(WiFi.status() != WL_CONNECTED){
        WiFi.begin(ssid, password); 
        delay(5000);     
      } 
      Serial.println("\nConnected.");
    }

    // Get a new temperature reading
    temperatureC = bme.readTemperature();
    Serial.print("Temperature (ºC): ");
    Serial.println(temperatureC);
    
    //uncomment if you want to get temperature in Fahrenheit
    /*temperatureF = 1.8 * bme.readTemperature() + 32;
    Serial.print("Temperature (ºC): ");
    Serial.println(temperatureF);*/
    
    
    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeField(myChannelNumber, 1, temperatureC, myWriteAPIKey);
    //uncomment if you want to get temperature in Fahrenheit
    //int x = ThingSpeak.writeField(myChannelNumber, 1, temperatureF, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel update successful.");
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
    lastTime = millis();
  }
}