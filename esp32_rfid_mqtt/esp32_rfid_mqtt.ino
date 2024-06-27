#include <PubSubClient.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  5  // ESP32 pin GPIO5 
#define RST_PIN 27 // ESP32 pin GPIO27 

MFRC522 rfid(SS_PIN, RST_PIN);
String uidString = ""; // Initialize an empty string to store the UID


// WiFi
const char* ssid = "KarthikT";                
const char* wifi_password = "deathnote";

// MQTT
const char* mqtt_server = "192.168.25.62"; 

const char* rfid_topic = "rfid";
const char* mqtt_username = "RPi"; // MQTT username
const char* mqtt_password = "RPi"; // MQTT password
const char* clientID = "Weather_Reporter"; // MQTT client ID

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;

// 1883 is the listener port for the Broker
PubSubClient client(mqtt_server, 1883, wifiClient);

 

// Custom function to connect to the MQTT broker via WiFi
void connect_MQTT(){
  Serial.print("Connecting to ");
  Serial.println(ssid);

  // Connect to the WiFi
  WiFi.begin(ssid, wifi_password);

  // Wait until the connection is confirmed
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Debugging – Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // Connect to MQTT Broker
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    Serial.println("Connected to MQTT Broker!");
  }
  else {
    Serial.println("Connection to MQTT Broker failed…");
  }
}

void setup() {
  Serial.begin(9600);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");



}

void loop() {
  connect_MQTT();
  Serial.setTimeout(2000);
  uidString = "";
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

      for (int i = 0; i < rfid.uid.size; i++) {
        if (rfid.uid.uidByte[i] < 0x10) {
          uidString += "0"; // Add leading zero if necessary
    }
        uidString += String(rfid.uid.uidByte[i], HEX); // Convert byte to hexadecimal and add to the string
        uidString += " "; // Add a space between bytes
  }

  // Print the UID
      Serial.print("UID: ");
      Serial.println(uidString);
      }
      Serial.println();

      rfid.PICC_HaltA(); // halt PICC
      rfid.PCD_StopCrypto1(); // stop encryption on PCD
    }


  String rf="RFID: "+String(uidString)+"  ";

  // PUBLISH to the MQTT Broker (topic = Temperature)
  if (client.publish(rfid_topic, String(uidString).c_str())) {
    Serial.println("RFID sent!");
  }
  else {
    Serial.println("RFID failed to send.Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesnt clash with the client.connect call
    client.publish(rfid_topic, String(uidString).c_str());
  }

  
  client.disconnect();  // disconnect from the MQTT broker
  delay(1000*3);       
}