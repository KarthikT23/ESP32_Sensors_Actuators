HardwareSerial mySerial(1);  // Use Serial1 for UART communication

void setup() {
  Serial.begin(115200);  // Initialize serial communication at 115200 bps for debugging
  mySerial.begin(9600, SERIAL_8N1, 16, 17);  // Initialize Serial1 at 9600 bps (pins 16 and 17)
}

void loop() {
  if (mySerial.available()) {
    String message = mySerial.readStringUntil('\n');  // Read data from Arduino Uno
    Serial.println("Received from Arduino Uno: " + message);  // Print received data
  }
}
