#include <DHT.h>
#include <PubSubClient.h>
#include <WiFi.h>

#define DHTPIN 4 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);

// WiFi
const char* ssid = "KarthikT";                
const char* wifi_password = "deathnote";

// MQTT
const char* mqtt_server = "192.168.25.62"; 
const char* humidity_topic = "humidity";
const char* temperature_topic = "temperature";
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
  dht.begin();
}

void loop() {
  connect_MQTT();
  Serial.setTimeout(2000);

  float h = dht.readHumidity();
  float t = dht.readTemperature();

  Serial.print("Humidity: ");
  Serial.print(h);
  Serial.println("%");
  Serial.print("Temperature:");
  Serial.print(t);
  Serial.println(" *C");

  // MQTT can only transmit strings
  String hs="Hum: "+String((float)h)+" % ";
  String ts="Temp:"+ String((float)t)+ +"C";

  // PUBLISH to the MQTT Broker (topic = Temperature)
  if (client.publish(temperature_topic, String(t).c_str())) {
    Serial.println("Temperature sent!");
  }
  else {
    Serial.println("Temperature failed to send.Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesnt clash with the client.connect call
    client.publish(temperature_topic, String(t).c_str());
  }

  // PUBLISH to the MQTT Broker (topic = Humidity)
  if (client.publish(humidity_topic, String(h).c_str())) {
    Serial.println("Humidity sent!");
  }
  else {
    Serial.println("Humidity failed to send. Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesn’t clash with the client.connect call
    client.publish(humidity_topic, String(h).c_str());
  }
  client.disconnect();  // disconnect from the MQTT broker
  delay(1000*60);       // print new values after 1 Minute
}