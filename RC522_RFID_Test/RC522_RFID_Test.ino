//RFID RC522 Pins	  ESP32 Pins
//  VCC	              +3.3V
//  RST	                22
//  GND	               GND
//  MISO	              19
//  MOSI	              23
//  SCK	                18
//  SS/SDA	            5


#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN  5  // ESP32 pin GPIO5 
#define RST_PIN 22 // ESP32 pin GPIO27 

MFRC522 rfid(SS_PIN, RST_PIN);
String uidString = ""; // Initialize an empty string to store the UID
void setup() {
  Serial.begin(9600);
  SPI.begin(); // init SPI bus
  rfid.PCD_Init(); // init MFRC522

  Serial.println("Tap an RFID/NFC tag on the RFID-RC522 reader");
}

void loop() {
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
  }

