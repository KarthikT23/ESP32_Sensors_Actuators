#include <PubSubClient.h>
#include <WiFi.h>
#include <Wire.h>
#include <BH1750.h>

BH1750 lightMeter;



// WiFi
const char* ssid = "KarthikT";                
const char* wifi_password = "deathnote";

// MQTT
const char* mqtt_server = "192.168.25.62"; 

const char* light_level_topic = "light level";
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
 
  Wire.begin();
  // On esp8266 you can select SCL and SDA pins using Wire.begin(D4, D3);
  // For Wemos / Lolin D1 Mini Pro and the Ambient Light shield use Wire.begin(D2, D1);

  lightMeter.begin();

  Serial.println(F("BH1750 Test begin"));
}

void loop() {
  connect_MQTT();
  Serial.setTimeout(2000);


  float lux = lightMeter.readLightLevel();


  Serial.println(" *C");
  Serial.print("Light: ");
  Serial.print(lux);
  Serial.println(" lx");
  delay(1000);

  // MQTT can only transmit strings
  //String hs="Hum: "+String((float)h)+" % ";
  //String ts="Temp:"+ String((float)t)+ +"C";
  String ll="Light: "+String((float)lux)+" lux ";

  // PUBLISH to the MQTT Broker (topic = Temperature)
  if (client.publish(light_level_topic, String(lux).c_str())) {
    Serial.println("Light level sent!");
  }
  else {
    Serial.println("Light level failed to send.Reconnecting to MQTT Broker and trying again");
    client.connect(clientID, mqtt_username, mqtt_password);
    delay(10); // This delay ensures that client.publish doesnt clash with the client.connect call
    client.publish(light_level_topic, String(lux).c_str());
  }

  
  client.disconnect();  // disconnect from the MQTT broker
  delay(1000);       // print new values after 1 Minute
}