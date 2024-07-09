#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "DHT.h"
#include <WiFi.h>

// define the BLE server name
#define bleServerName "ESP32_DHT11"

// define the Service UUID as a string
#define SERVICE_UUID "00000002-0000-0000-FDFD-FDFDFDFDFDFD"

// Temperature Characteristic and Descriptor (default UUID from https://www.bluetooth.com/specifications/gss/)
BLECharacteristic temperatureCharacteristic(BLEUUID((uint16_t)0x2A6E), BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor temperatureDescriptor(BLEUUID((uint16_t)0x2902));

// Humidity Characteristic and Descriptor (default UUID from https://www.bluetooth.com/specifications/gss/)
BLECharacteristic humidityCharacteristic(BLEUUID((uint16_t)0x2A6F), BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor humidityDescriptor(BLEUUID((uint16_t)0x2902));

// Text Characteristic for receiving text ie SSID,Password of your WiFi
BLECharacteristic textCharacteristic(BLEUUID((uint16_t)0x2A3D), BLECharacteristic::PROPERTY_WRITE);

// Define the DHT sensor pins and type
#define DHTPIN 26
#define DHTTYPE DHT11
// Create a sensor object
DHT dht(DHTPIN, DHTTYPE);

bool deviceConnected = false;
bool wifiConnected = false;

// WiFi credentials
String ssid;
String password;

// Setup callbacks onConnect and onDisconnect for BLE
class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
    Serial.println("Device Connected");
  };
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    Serial.println("Device Disconnected");
  }
};

// Callbacks for handling received text (SSID and password)
class TextCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue().c_str();
    if (value.length() > 0) {
      Serial.print("Received text: ");
      Serial.println(value.c_str());

      // Extract SSID and password from received string
      String receivedText = String(value.c_str());
      int separatorIndex = receivedText.indexOf(',');
      if (separatorIndex != -1) {
        ssid = receivedText.substring(0, separatorIndex);
        password = receivedText.substring(separatorIndex + 1);
        Serial.print("SSID: ");
        Serial.println(ssid);
        Serial.print("Password: ");
        Serial.println(password);

        // Attempt to connect to WiFi
        WiFi.begin(ssid.c_str(), password.c_str());
        Serial.print("Connecting to WiFi...");
        while (WiFi.status() != WL_CONNECTED) {
          delay(1000);
          Serial.print(".");
        }
        Serial.println("");
        
        if (WiFi.status() == WL_CONNECTED) {
          Serial.println("WiFi connected");
          wifiConnected = true;
        } else {
          Serial.println("WiFi connection failed");
        }
      }
    }
  }
};

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Create the BLE Device
  BLEDevice::init(bleServerName);

  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *dhtService = pServer->createService(BLEUUID(SERVICE_UUID));

  // Create Temperature Characteristic
  temperatureCharacteristic.addDescriptor(new BLE2902());
  dhtService->addCharacteristic(&temperatureCharacteristic);

  // Create Humidity Characteristic
  humidityCharacteristic.addDescriptor(new BLE2902());
  dhtService->addCharacteristic(&humidityCharacteristic);

  // Create Text Characteristic
  textCharacteristic.setCallbacks(new TextCallbacks());
  dhtService->addCharacteristic(&textCharacteristic);

  // Start the service
  dhtService->start();

  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting for a client connection to notify...");
}

void loop() {
  if (deviceConnected) {
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read humidity
    float h = dht.readHumidity();

    // Check if any reads failed and exit early (to try again)
    if (isnan(t) || isnan(h)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Notify temperature reading
    uint16_t temperature = (uint16_t)t;
    temperatureCharacteristic.setValue((uint8_t*)&temperature, sizeof(temperature));
    temperatureCharacteristic.notify();
    Serial.print("Temperature Celsius: ");
    Serial.print(t);
    Serial.println(" ºC");

    // Notify humidity reading
    uint16_t humidity = (uint16_t)h;
    humidityCharacteristic.setValue((uint8_t*)&humidity, sizeof(humidity));
    humidityCharacteristic.notify();
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");

    delay(5000); // Update every 5 seconds
  }
}
