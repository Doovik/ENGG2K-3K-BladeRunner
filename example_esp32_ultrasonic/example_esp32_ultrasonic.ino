#include <NewPing.h>
#include <WiFi.h>

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

WiFiClient client;

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

  // Connect to the server
  if (client.connect(server_ip, server_port)) {
    Serial.println("Connected to server");
  } else {
    Serial.println("Connection to server failed");
  }
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

  // Check if connected to the server
  if (client.connected()) {
    // Send data to the server
    client.print("S1: ");
    client.print(S1_distance);
    client.print("cm, S2: ");
    client.print(S2_distance);
    client.println("cm");

    // Wait for a response from the server
    while (client.available()) {
      String response = client.readStringUntil('\n');
      Serial.println("Received from server: " + response);
    }
  } else {
    Serial.println("Disconnected from server");
    // Attempt to reconnect
    if (client.connect(server_ip, server_port)) {
      Serial.println("Reconnected to server");
    }
  }

  delay(10);  // Wait before starting the next loop
}