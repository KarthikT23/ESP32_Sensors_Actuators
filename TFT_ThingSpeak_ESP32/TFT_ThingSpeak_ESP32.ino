#include <WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>
#include <BH1750.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7735.h>
#include <Adafruit_ST7789.h>
#include <SPI.h>

// WiFi credentials
const char* ssid = "KarthikT";   
const char* password = "deathnote";   

WiFiClient client;

// ThingSpeak channel information
unsigned long myChannelNumber = 2591745; 
const char *myWriteAPIKey = "ZOO29DAHVNH575CM";

unsigned long lastTime = 0;
unsigned long timerDelay = 30000; // Interval between updates (in milliseconds)

// Define DHT sensor pin and type
#define DHTPIN 26
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

BH1750 lightMeter;

Adafruit_BMP085 bmp;

//const int mq2Pin = 15; // Analog pin for MQ2 sensor

float Temperature, Humidity, HeatIndex, AmbientLight, Pressure, Altitude, SeaLevelPressure;

// TFT display configuration
#define TFT_CS   13
#define TFT_RST  0 
#define TFT_DC   12
#define TFT_MOSI 14
#define TFT_SCLK 27

Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

void setup() {
  Serial.begin(115200); 
  delay(1000); 

  // Initialize TFT display
  tft.initR(INITR_BLACKTAB);      
  tft.fillScreen(ST77XX_BLACK);
  tft.setTextColor(ST77XX_WHITE);
  tft.setTextSize(1);

  // Initialize sensors
  initDHT();
  initBMP180();

  // Initialize WiFi and ThingSpeak
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);

  // Initialize BH1750 light sensor
  Wire.begin();
  lightMeter.begin();
}

void loop() {
  if ((millis() - lastTime) > timerDelay) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.print("Attempting to connect");
      while (WiFi.status() != WL_CONNECTED) {
        WiFi.begin(ssid, password);
        delay(5000);
      }
      Serial.println("\nConnected.");
    }

    // Read sensor data
    readDHT();
    readBH1750();
    readBMP180();

    // Display sensor data on TFT
    displaySensorData();

    // Write data to ThingSpeak fields
    ThingSpeak.setField(1, Temperature);
    ThingSpeak.setField(2, Humidity);
    ThingSpeak.setField(3, HeatIndex);
    ThingSpeak.setField(4, AmbientLight);
    ThingSpeak.setField(5, Pressure);
    ThingSpeak.setField(6, Altitude);
    ThingSpeak.setField(7, SeaLevelPressure);
    int result = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (result == 200) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(result));
    }
    lastTime = millis();
  }
}

void initDHT() {
  dht.begin();
  Serial.println("DHT sensor initialized.");
}

void readDHT() {
  Humidity = dht.readHumidity();
  Temperature = dht.readTemperature();
  HeatIndex = dht.computeHeatIndex(Temperature, Humidity, false);
  Serial.print("Humidity: ");
  Serial.print(Humidity);
  Serial.print("% Temperature: ");
  Serial.print(Temperature);
  Serial.print("°C Heat index: ");
  Serial.println(HeatIndex);
}

void readBH1750() {
  AmbientLight = lightMeter.readLightLevel();
  Serial.print("Light: ");
  Serial.print(AmbientLight);
  Serial.println(" lx");
}

void initBMP180() {
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");
    while (1);
  }
  Serial.println("BMP180 sensor initialized.");
}

void readBMP180() {
  Pressure = bmp.readPressure();
  Altitude = bmp.readAltitude();
  SeaLevelPressure = bmp.readSealevelPressure();
  Serial.print("Pressure: ");
  Serial.print(Pressure);
  Serial.print(" Pa Altitude: ");
  Serial.print(Altitude);
  Serial.print(" meters Sea level pressure: ");
  Serial.println(SeaLevelPressure);
}

void displaySensorData() {
  tft.fillScreen(ST77XX_BLACK);
  tft.setCursor(0, 0);

  tft.print("Temp: "); tft.println(Temperature);
  tft.print("Hum: "); tft.println(Humidity);
  tft.print("HeatIdx: "); tft.println(HeatIndex);
  tft.print("Light: "); tft.println(AmbientLight);
  tft.print("Pressure: "); tft.println(Pressure);
  tft.print("Altitude: "); tft.println(Altitude);
  tft.print("SeaLvlPress: "); tft.println(SeaLevelPressure);
}
