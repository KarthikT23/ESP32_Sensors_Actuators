#include <WiFi.h>
#include <ThingSpeak.h>
#include <DHT.h>
#include <BH1750.h>
#include <Wire.h>
#include <Adafruit_BMP085.h>

const char* ssid = "KarthikT";   // your network SSID (name) 
const char* password = "deathnote";   // your network password

WiFiClient client;

unsigned long myChannelNumber = 2591745; // Replace with your ThingSpeak channel number
const char *myWriteAPIKey = "ZOO29DAHVNH575CM"; // Replace with your ThingSpeak write API key

unsigned long lastTime = 0;
unsigned long timerDelay = 30000; // Interval between updates (in milliseconds)

// Define DHT sensor pin and type
#define DHTPIN 4
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

BH1750 lightMeter;

Adafruit_BMP085 bmp;

const int mq2Pin = 15; // Analog pin for MQ2 sensor

float Temperature, Humidity, HeatIndex, AmbientLight, Pressure, Altitude, SeaLevelPressure, GasConcentration;

void setup() {
  Serial.begin(115200); // Initialize serial communication
  delay(1000); // Delay to let serial console settle
  
  // Initialize sensors
  initDHT();
  initBH1750();
  initBMP180();

  // Initialize WiFi and ThingSpeak
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
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
    readMQ2();

    // Write data to ThingSpeak fields
    //float fields[8] = {Temperature, Humidity, HeatIndex, AmbientLight, Pressure, Altitude, SeaLevelPressure, GasConcentration};
    //int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey, fields, 8);
    int t = ThingSpeak.setField(1, Temperature);
    int h = ThingSpeak.setField(2, Humidity);
    int hi = ThingSpeak.setField(3, HeatIndex);
    int al = ThingSpeak.setField(4, AmbientLight);
    int p = ThingSpeak.setField(5, Pressure);
    int a = ThingSpeak.setField(6, Altitude);
    int slp = ThingSpeak.setField(7, SeaLevelPressure);
    int gc = ThingSpeak.setField(8, GasConcentration);
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
    if (true) {
      Serial.println("Channel update successful.");
    } else {
      Serial.println("Problem updating channel. HTTP error code " + String(t)+ String(h)+ String(hi)+ String(al)+ String(p)+ String(a)+ String(slp)+ String(gc));
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

void initBH1750() {
  lightMeter.begin();
  Serial.println("BH1750 sensor initialized.");
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

void readMQ2() {
  int sensorValue = analogRead(mq2Pin);
  // Convert the analog reading to gas concentration
  //GasConcentration = map(sensorValue, 0, 10000, 0, 100); // Example mapping, adjust based on sensor characteristics
  GasConcentration = analogRead(mq2Pin);
  Serial.print("Gas concentration: ");
  Serial.print(GasConcentration);
  Serial.println(" %");
}
