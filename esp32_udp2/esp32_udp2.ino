#include <WiFi.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  5  // ESP32 pin GPIO5 
#define RST_PIN 27 // ESP32 pin GPIO27 

MFRC522 rfid(SS_PIN, RST_PIN);
String uidString = ""; // Initialize an empty string to store the UID

const char* ssid = "KarthikT";
const char* password = "deathnote";

const char* udp_server_ip = "192.168.72.62";
unsigned int udp_server_port = 12345;  // Port on which Raspberry Pi is listening

WiFiUDP udp;

void setup() {
  Serial.begin(115200);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
  // Prepare and send a message to the Raspberry Pi
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
 if (!uidString.isEmpty()) {
    udp.beginPacket(udp_server_ip, udp_server_port);
    udp.print(uidString);
    udp.endPacket();
    Serial.println("Sent RFID: " + uidString);
  
  }
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
