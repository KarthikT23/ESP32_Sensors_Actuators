#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "DHT.h"

// BLE server name
#define bleServerName "ESP32_DHT11"

// Default UUID for Environmental Sensing Service
#define SERVICE_UUID (BLEUUID((uint16_t)0x181A))

// Temperature Characteristic and Descriptor (default UUID)
BLECharacteristic temperatureCharacteristic(BLEUUID((uint16_t)0x2A6E), BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor temperatureDescriptor(BLEUUID((uint16_t)0x2902));

// Humidity Characteristic and Descriptor (default UUID)
BLECharacteristic humidityCharacteristic(BLEUUID((uint16_t)0x2A6F), BLECharacteristic::PROPERTY_NOTIFY);
BLEDescriptor humidityDescriptor(BLEUUID((uint16_t)0x2902));

#define DHTPIN 26
#define DHTTYPE DHT11

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
  BLEService *dhtService = pServer->createService(SERVICE_UUID);

  // Create BLE Characteristics and corresponding Descriptors
  dhtService->addCharacteristic(&temperatureCharacteristic);
  temperatureCharacteristic.addDescriptor(&temperatureDescriptor);

  dhtService->addCharacteristic(&humidityCharacteristic);
  humidityCharacteristic.addDescriptor(&humidityDescriptor);

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
