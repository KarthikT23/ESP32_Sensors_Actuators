#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "DHT.h"

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

// Text Characteristic for receiving text
BLECharacteristic textCharacteristic(BLEUUID((uint16_t)0x2A3D), BLECharacteristic::PROPERTY_WRITE);

// Define the DHT sensor pins and type
#define DHTPIN 26
#define DHTTYPE DHT11
// Create a sensor object
DHT dht(DHTPIN, DHTTYPE);

bool deviceConnected = false;

// Setup callbacks onConnect and onDisconnect
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

class TextCallbacks: public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    std::string value = pCharacteristic->getValue().c_str();
    if (value.length() > 0) {
      Serial.print("Received text: ");
      Serial.println(value.c_str());
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

  // Create BLE Characteristics and corresponding Descriptors
  dhtService->addCharacteristic(&temperatureCharacteristic);
  temperatureCharacteristic.addDescriptor(&temperatureDescriptor);

  dhtService->addCharacteristic(&humidityCharacteristic);
  humidityCharacteristic.addDescriptor(&humidityDescriptor);

  // Add the text characteristic and set its callback
  dhtService->addCharacteristic(&textCharacteristic);
  textCharacteristic.setCallbacks(new TextCallbacks());

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
    // Set temperature Characteristic value and notify connected client
    temperatureCharacteristic.setValue(temperature);
    temperatureCharacteristic.notify();
    Serial.print("Temperature Celsius: ");
    Serial.print(t);
    Serial.println(" ºC");

    // Notify humidity reading
    uint16_t humidity = (uint16_t)h;
    // Set humidity Characteristic value and notify connected client
    humidityCharacteristic.setValue(humidity);
    humidityCharacteristic.notify();
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.println(" %");

    delay(2000); // Update every 2 seconds
  }
}
