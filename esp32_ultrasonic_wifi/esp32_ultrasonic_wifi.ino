#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>

// WiFi credentials
const char* ssid = "";
const char* password = "";

// Static IP address configuration
IPAddress local_IP(192, 168, 0, 0); //ESP IP
IPAddress gateway(192, 168, 0, 0);
IPAddress subnet(255, 255, 255, 0);

// Server IP and port
const char* server_ip = "192.168.0.0";  // Replace with the IP address of your Java server
const uint16_t server_port = 3014;

WiFiUDP udp;

// Variable to store the most recent packet received
String mostRecentPacket = "";

void setup()
{
  Serial.begin(9600);

  // Connect to Wi-Fi
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  // Send "I am alive" message
  udp.beginPacket(server_ip, server_port);
  udp.println("I am alive");
  udp.endPacket();
  Serial.println("Sent: I am alive");
}

void loop()
{
  // Check for incoming UDP packets
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    Serial.printf("Received from server: %s\n", incomingPacket);

    // Store the most recent packet
    mostRecentPacket = String(incomingPacket);

    // Parse the received message
    char* token = strtok(incomingPacket, ",");
    int sequenceId = atoi(token);
    token = strtok(NULL, ",");
    String action = String(token);

    Serial.printf("Sequence ID: %d, Action: %s\n", sequenceId, action.c_str());
  } else {
    Serial.println("No packet received");
  }

  // Send a message to the server
  udp.beginPacket(server_ip, server_port);
  udp.endPacket();

  delay(1000);  
}