#include <WiFi.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <NewPing.h>

// WiFi credentials
const char* ssid = "ENGG2K3K";
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

// CCP status
String ccpStatus = "STOPC"; // Example status, replace with actual status

// Sequence numbers
int execSequenceNumber = 2;
int strqSequenceNumber = 1;
int lastExecSequenceNumber = 0; // To track the highest EXEC sequence number


const int S1_TRIGGER_PIN = 25;
const int S1_ECHO_PIN = 26;
const int S2_TRIGGER_PIN = 27;
const int S2_ECHO_PIN = 14;

const int S1_RED_PIN = 16;
const int S1_GREEN_PIN = 17;
const int S2_RED_PIN = 18;
const int S2_GREEN_PIN = 19;

const int pwmPin = 33;
const int dirPin1 = 2;
const int dirPin2 = 15;

const int maxDist = 450;
const int minBrakingDist = 1;
const int maxBrakingDist = 5;

NewPing S1_sonar(S1_TRIGGER_PIN, S1_ECHO_PIN, maxDist);
NewPing S2_sonar(S2_TRIGGER_PIN, S2_ECHO_PIN, maxDist);

void setup()
{
  Serial.begin(9600);
  pinMode(S1_RED_PIN, OUTPUT);
  pinMode(S1_GREEN_PIN, OUTPUT);
  pinMode(S2_RED_PIN, OUTPUT);
  pinMode(S2_GREEN_PIN, OUTPUT);
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);

  // Connect to Wi-Fi
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to WiFi");

  // Send "I am alive" message until confirmation is received
  while (true) {
    // Send "I am alive" message
    udp.beginPacket(server_ip, server_port);
    udp.println("I am alive");
    udp.endPacket();
    Serial.println("Sent: I am alive");

    // Wait for a short period before checking for a response
    delay(1000);

    // Check for incoming UDP packets
    int packetSize = udp.parsePacket();
    if (packetSize) {
      char incomingPacket[255];
      int len = udp.read(incomingPacket, 255);
      if (len > 0) {
        incomingPacket[len] = 0;
      }
      Serial.printf("Received from CCP: %s\n", incomingPacket);

      // Store the most recent packet
      mostRecentPacket = String(incomingPacket);

      // Check if the message is "ACK"
      if (mostRecentPacket.equals("ACK")) {
        Serial.println("Received 'ACK' message from server.");
        break; // Exit the loop if confirmation is received
      }
    }
  }
}

void stopBladeRunner() {
  analogWrite(pwmPin, 0);
  Serial.println("Blade Runner has Stopped.");
}

void goForwardSlow() {
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);
  analogWrite(pwmPin, 64);
  Serial.println("Moving slowly forward.");
}

void goForwardFast() {
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);
  analogWrite(pwmPin, 150);
  Serial.println("Moving fast forward.");
}

void reverseSlow() {
  digitalWrite(dirPin1, LOW);
  digitalWrite(dirPin2, HIGH);
  analogWrite(pwmPin, 64);
  Serial.println("Moving slowly backwards.");
}

void checkEmergencyBraking()  {
  int distance = 0;
  if(ccpStatus == "RSLOWC") {
    distance = S2_sonar.ping_cm();
    Serial.print("Rear sensor distance: ");
    Serial.println(distance);
  } else {
    distance = S1_sonar.ping_cm();
    Serial.print("Front sensor distance: ");
    Serial.println(distance);
  }

  // If distance is between 1 and 5 cm, trigger an emergency stop
  if (distance >= minBrakingDist && distance <= maxBrakingDist) {
    stopBladeRunner();
    Serial.println("Emergency Stop: Object detected within 1-5 cm.");
  }
}

void handleCommand(const char* action) {
  if (strcmp(action, "STOPC") == 0) {
    stopBladeRunner();
    ccpStatus = "STOPC";
  } else if (strcmp(action, "STOPO") == 0) {
    stopBladeRunner();
    ccpStatus = "STOPO";
  } else if (strcmp(action, "FSLOWC") == 0) {
    goForwardSlow();
    ccpStatus = "FSLOWC";
  } else if (strcmp(action, "FFASTC") == 0) {
    goForwardFast();
    ccpStatus = "FFASTC";
  } else if (strcmp(action, "RSLOWC") == 0) {
    reverseSlow();
    ccpStatus = "RSLOWC";
  } else if (strcmp(action, "DISCONNECT") == 0) {
    stopBladeRunner();
    ccpStatus = "OFLN";
  }
}

void loop() {

  checkEmergencyBraking();

  // Check for incoming UDP packets
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }
    Serial.printf("Received from CCP: %s\n", incomingPacket);

    // Parse the received message
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, incomingPacket);
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.f_str());
      return;
    }

    const char* messageType = doc["message"];
    const char* clientId = doc["client_id"];
    const char* sequenceNumber = doc["sequence_number"];

    if (strcmp(messageType, "STRQ") == 0) {
      // Handle MCP Status Request Message
      StaticJsonDocument<256> responseDoc;
      responseDoc["client_type"] = "CCP";
      responseDoc["message"] = "STAT";
      responseDoc["client_id"] = clientId;
      responseDoc["sequence_number"] = String(strqSequenceNumber).c_str();
      responseDoc["status"] = ccpStatus;

      char responseBuffer[256];
      size_t responseSize = serializeJson(responseDoc, responseBuffer);

      udp.beginPacket(server_ip, server_port);
      udp.write((const uint8_t*)responseBuffer, responseSize); // Cast to const uint8_t*
      udp.endPacket();
      Serial.printf("Sent to CCP: %s\n", responseBuffer);

      // Increment the sequence number for STRQ messages
      strqSequenceNumber++;
    } else if (strcmp(messageType, "EXEC") == 0) {
      // Handle MCP Command Message
      const char* action = doc["action"];
      int incomingSequenceNumber = atoi(sequenceNumber);

      // Check if the incoming EXEC message has a higher sequence number
      if (incomingSequenceNumber > lastExecSequenceNumber) {
        handleCommand(action);
        // Update the most recent packet and the last EXEC sequence number
        mostRecentPacket = String(incomingPacket);
        lastExecSequenceNumber = incomingSequenceNumber;
      }

      StaticJsonDocument<256> responseDoc;
      responseDoc["client_type"] = "CCP";
      responseDoc["message"] = "AKEX";
      responseDoc["client_id"] = clientId;
      responseDoc["sequence_number"] = String(execSequenceNumber).c_str();

      char responseBuffer[256];
      size_t responseSize = serializeJson(responseDoc, responseBuffer);

      udp.beginPacket(server_ip, server_port);
      udp.write((const uint8_t*)responseBuffer, responseSize); // Cast to const uint8_t*
      udp.endPacket();
      Serial.printf("Sent to CCP: %s\n", responseBuffer);

      // Increment the sequence number for EXEC messages
      execSequenceNumber++;
    }

    // Print the most recent packet
    Serial.printf("Most recent packet: %s\n", mostRecentPacket.c_str());
  } else {
    Serial.println("No packet received");
  }

  delay(500);  
}