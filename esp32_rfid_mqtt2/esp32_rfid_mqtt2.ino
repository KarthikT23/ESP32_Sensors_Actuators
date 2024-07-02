#include <PubSubClient.h>
#include <WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define SS_PIN  5  // ESP32 pin GPIO5 
#define RST_PIN 27 // ESP32 pin GPIO27 

MFRC522 rfid(SS_PIN, RST_PIN);
String uidString = ""; // Initialize an empty string to store the UID


// WiFi
const char* ssid = "KarthikT";                
const char* wifi_password = "deathnote";

// MQTT
const char* mqtt_server = "192.168.72.62"; 

const char* rfid_topic = "rfid";
const char* mqtt_username = "RPi"; // MQTT username
const char* mqtt_password = "RPi"; // MQTT password
const char* clientID = "Weather_Reporter"; // MQTT client ID
const char* rfid_response_topic = "rfid_response";

// Initialise the WiFi and MQTT Client objects
WiFiClient wifiClient;

// 1883 is the listener port for the Broker
PubSubClient client(mqtt_server, 1883, wifiClient);

const char* udp_server_ip = "192.168.72.62";
unsigned int udp_server_port = 12345;  // Port on which Raspberry Pi is listening

WiFiUDP udp;

 

// Custom function to connect to the MQTT broker via WiFi
void connect_MQTT(){
  //Serial.print("Connecting to ");
  //Serial.println(ssid);

  // Connect to the WiFi
  WiFi.begin(ssid, wifi_password);

  // Wait until the connection is confirmed
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }

  // Debugging – Output the IP Address of the ESP8266
  //Serial.println("WiFi connected");
  //Serial.print("IP address: ");
  //Serial.println(WiFi.localIP());

  // Connect to MQTT Broker
  if (client.connect(clientID, mqtt_username, mqtt_password)) {
    //Serial.println("Connected to MQTT Broker!");
  }
  else {
    //Serial.println("Connection to MQTT Broker failed…");
  }
}

void setup() {
  Serial.begin(115200);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522
  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  WiFi.begin(ssid, wifi_password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop2() {
  // Prepare and send a message to the Raspberry Pi
  String message = "Hello from ESP32";
  udp.beginPacket(udp_server_ip, udp_server_port);
  udp.print(message);
  udp.endPacket();
  Serial.println("Sent: " + message);

  // Wait for a response from the Raspberry Pi
  delay(1000);  // Adjust as needed
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char packetBuffer[255]; // Adjust buffer size as needed
    udp.read(packetBuffer, packetSize);
    packetBuffer[packetSize] = '\0'; // Null-terminate the received data
    Serial.println("Received: " + String(packetBuffer));

    // Process the received data as needed
  }

  // Other loop logic here
}

void loop() {
  connect_MQTT();
  Serial.setTimeout(2000);
  uidString = "";
  if (rfid.PICC_IsNewCardPresent()) { // new tag is available
    if (rfid.PICC_ReadCardSerial()) { // NUID has been read
      MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
      Serial.print("RFID/NFC Tag Type: ");
      Serial.println(rfid.PICC_GetTypeName(piccType));

    for (int i = 0; i < rfid.uid.size; i++) {
      if (rfid.uid.uidByte[i] < 0x10) {
        uidString += "0"; // Add leading zero if necessary
      }
      uidString += String(rfid.uid.uidByte[i], HEX); // Convert byte to hexadecimal and add to the string
      if (i < rfid.uid.size - 1) {
        uidString += " "; // Add a space between bytes, but not after the last byte
      }
    }


      // Print the UID
      Serial.print("UID: ");
      Serial.println(uidString);

      String rf = "RFID: " + String(uidString) + " ";

      // PUBLISH to the MQTT Broker (topic = Temperature)
      if (client.publish(rfid_topic, String(uidString).c_str())) {
        Serial.println("RFID sent!");
      } else {
        Serial.println("RFID failed to send. Reconnecting to MQTT Broker and trying again");
        client.connect(clientID, mqtt_username, mqtt_password);
        delay(10); // This delay ensures that client.publish doesn't clash with the client.connect call
        client.publish(rfid_topic, String(uidString).c_str());
      }
    }

    rfid.PICC_HaltA(); // halt PICC
    rfid.PCD_StopCrypto1(); // stop encryption on PCD
  }

  client.disconnect();  // disconnect from the MQTT broker
  delay(1000 * 3);
  loop2();
}
