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

// CCP status
String ccpStatus = "STOPC"; // Example status, replace with actual status

// Sequence numbers
int execSequenceNumber = 2;
int strqSequenceNumber = 1;
int lastExecSequenceNumber = 0; // To track the highest EXEC sequence number

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
        // Update the status based on the action
        if (strcmp(action, "STOPC") == 0) {
          ccpStatus = "STOPC";
        } else if (strcmp(action, "STOPO") == 0) {
          ccpStatus = "STOPO";
        } else if (strcmp(action, "FSLOWC") == 0) {
          ccpStatus = "FSLOWC";
        } else if (strcmp(action, "FFASTC") == 0) {
          ccpStatus = "FFASTC";
        } else if (strcmp(action, "RSLOWC") == 0) {
          ccpStatus = "RSLOWC";
        } else if (strcmp(action, "DISCONNECT") == 0) {
          ccpStatus = "OFLN";
        }

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

  delay(1000);  
}