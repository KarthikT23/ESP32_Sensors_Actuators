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
unsigned int udp_server_port = 12345;  // 192.168.25.6Port on which Raspberry Pi is listening

WiFiUDP udp;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
}

void loop() {
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