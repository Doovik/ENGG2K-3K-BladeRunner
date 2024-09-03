#include <NewPing.h>
#include <WiFi.h>
#include <WiFiUdp.h>

// WiFi credentials
const char* ssid = "your_SSID";
const char* password = "your_PASSWORD";

// Static IP address configuration
IPAddress local_IP(192, 168, 1, 184);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

// Server IP and port
const char* server_ip = "10.20.30.114";
const uint16_t server_port = 3014;

WiFiUDP udp;

#define S1_TRIGGER_PIN 25
#define S1_ECHO_PIN 26
#define S2_TRIGGER_PIN 27
#define S2_ECHO_PIN 14

#define maxDist 450

NewPing S1_sonar(S1_TRIGGER_PIN, S1_ECHO_PIN, maxDist);
NewPing S2_sonar(S2_TRIGGER_PIN, S2_ECHO_PIN, maxDist);

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
}

void loop()
{
  int S1_distance = S1_sonar.ping_cm();
  delay(10);  // Wait for the first echo to dissipate
  int S2_distance = S2_sonar.ping_cm();

  if(S1_distance < 1) {
    Serial.println("S1 Unable to read");
  } else {
    Serial.print("S1: ");
    Serial.print(S1_distance);
    Serial.println("cm");
  }

  if(S2_distance < 1) {
    Serial.println("S2 Unable to read");
  } else {
    Serial.print("S2: ");
    Serial.print(S2_distance);
    Serial.println("cm");
  }

  // Send sensor data to the server
  udp.beginPacket(server_ip, server_port);
  udp.print("S1: ");
  udp.print(S1_distance);
  udp.print("cm, S2: ");
  udp.print(S2_distance);
  udp.println("cm");
  udp.endPacket();

  // Check for incoming UDP packets
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    Serial.printf("Received from server: %s\n", incomingPacket);
  }

  delay(1000);  // Wait before starting the next loop
}