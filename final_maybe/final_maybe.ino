#include <WiFi.h> // WiFi Library
#include <WiFiUdp.h> // WiFi Library
#include <ArduinoJson.h> // Json Interpretation
#include <NewPing.h> // US Sensor Library
#include <stdio.h>

// WiFi credentials
const char* ssid = "ENGG2K3K";
const char* password = "";

// Static IP address configuration
IPAddress local_IP(10, 20, 30, 143); //ESP IP
IPAddress gateway(10, 20, 30, 250);
IPAddress subnet(255, 255, 255, 0);

// Server IP and port
const char* server_ip = "10.20.30.142";  // Replace with the IP address of your Java server
const uint16_t server_port = 3014;

WiFiUDP udp;

// CCP status enum
enum CCPStatus {
  STOPC,
  STOPO,
  FSLOWC,
  FFASTC,
  RSLOWC,
  OFLN,
  UNKNOWN
};

// Variable to store the most recent packet received
String mostRecentPacket = "";

// CCP status
CCPStatus ccpStatus = STOPC; // Default status

// Function to convert string to CCPStatus enum
CCPStatus getStatusFromString(const String& statusStr) {
  if (statusStr == "STOPC") return STOPC;
  if (statusStr == "STOPO") return STOPO;
  if (statusStr == "FSLOWC") return FSLOWC;
  if (statusStr == "FFASTC") return FFASTC;
  if (statusStr == "RSLOWC") return RSLOWC;
  if (statusStr == "OFLN") return OFLN;
  return UNKNOWN;
}

// Function to convert CCPStatus enum back to string
const char* getStringFromStatus(CCPStatus status) {
  switch (status) {
    case STOPC: return "STOPC";
    case STOPO: return "STOPO";
    case FSLOWC: return "FSLOWC";
    case FFASTC: return "FFASTC";
    case RSLOWC: return "RSLOWC";
    case OFLN: return "OFLN";
    default: return "UNKNOWN";
  }
}

// Sequence numbers
int execSequenceNumber = 2;
int strqSequenceNumber = 1;
int lastExecSequenceNumber = 0; // To track the highest EXEC sequence number

const int S1_TRIGGER_PIN = 25;
const int S1_ECHO_PIN = 26;
const int S2_TRIGGER_PIN = 27;
const int S2_ECHO_PIN = 14;

const int pwmPin = 33;
const int dirPin1 = 2;
const int dirPin2 = 15;

const int maxDist = 450;
const int minBrakingDist = 1;
const int maxBrakingDist = 5;

const int IRSensorPin = 12;
const int IRThreshold = 500;

const int ledPins[4] = {19, 18, 17, 16};

NewPing S1_sonar(S1_TRIGGER_PIN, S1_ECHO_PIN, maxDist);
NewPing S2_sonar(S2_TRIGGER_PIN, S2_ECHO_PIN, maxDist);

void setup()
{
  Serial.begin(9600);
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  pinMode(IRSensorPin, INPUT);
  pinMode(ledPins[0], OUTPUT);
  pinMode(ledPins[1], OUTPUT);
  pinMode(ledPins[2], OUTPUT);
  pinMode(ledPins[3], OUTPUT);
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);

  // Connect to Wi-Fi
  WiFi.config(local_IP, gateway, subnet);
  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }


  // Send "I am alive" message until confirmation is received
  while (true) {
    // Send "I am alive" message
    udp.beginPacket(server_ip, server_port);
    udp.println("I am alive");
    udp.endPacket();

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

      // Store the most recent packet
      mostRecentPacket = String(incomingPacket);

      // Check if the message is "ACK"
      if (mostRecentPacket.equals("ACK")) {
        break; // Exit the loop if confirmation is received
      }
    }
  }
}

void disconnectFun() {
  while(true) {
    delay(250);
    digitalWrite(ledPins[2], LOW);
    delay(250);
    digitalWrite(ledPins[2], HIGH);
  }
}

void handleLEDs(int onLED) {
  for(int i = 0; i < (sizeof(ledPins) / sizeof(ledPins[0])); i++) {
    if(ledPins[i] == onLED) {
      digitalWrite(onLED, HIGH);
    } else {
      digitalWrite(ledPins[i], LOW);
    }
  }
}

void stopBladeRunner() {
  analogWrite(pwmPin, 0);
}

void goForwardSlow() {
  switch(ccpStatus) {
    case FFASTC:
      for(int dutyCycle = 150; dutyCycle > 64; dutyCycle--) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;
    case STOPC:
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, LOW);
      for(int dutyCycle = 0; dutyCycle < 64; dutyCycle++) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;
    case STOPO:
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, LOW);
      for(int dutyCycle = 0; dutyCycle < 64; dutyCycle++) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;
    case RSLOWC:
      for(int dutyCycle = 64; dutyCycle > 0; dutyCycle--) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, LOW);
      for(int dutyCycle = 0; dutyCycle < 64; dutyCycle++) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;
    case FSLOWC:
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, LOW);
      analogWrite(pwmPin, 64);
      break;
  }
}

void goForwardFast() {
  switch (ccpStatus) {
    case FFASTC:
      // Already fast, no soft start needed
      analogWrite(pwmPin, 150);
      break;

    case STOPC:
      digitalWrite(dirPin1, HIGH); // Forward direction
      digitalWrite(dirPin2, LOW);
      for (int dutyCycle = 0; dutyCycle < 150; dutyCycle++) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;
    case STOPO:
      digitalWrite(dirPin1, HIGH); // Forward direction
      digitalWrite(dirPin2, LOW);
      for (int dutyCycle = 0; dutyCycle < 150; dutyCycle++) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;
    case FSLOWC:
      // Soft start from stop or slow to fast
      digitalWrite(dirPin1, HIGH);
      digitalWrite(dirPin2, LOW);
      for (int dutyCycle = 64; dutyCycle < 150; dutyCycle++) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;

    case RSLOWC:
      // Soft stop from reverse to fast forward
      for (int dutyCycle = 64; dutyCycle > 0; dutyCycle--) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      digitalWrite(dirPin1, HIGH); // Forward direction
      digitalWrite(dirPin2, LOW);
      for (int dutyCycle = 0; dutyCycle < 150; dutyCycle++) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;
  }
}

void reverseSlow() {
  switch (ccpStatus) {
    case STOPC:
      digitalWrite(dirPin1, LOW); // Reverse direction
      digitalWrite(dirPin2, HIGH);
      for (int dutyCycle = 0; dutyCycle < 64; dutyCycle++) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;
    case STOPO:
      digitalWrite(dirPin1, LOW); // Reverse direction
      digitalWrite(dirPin2, HIGH);
      for (int dutyCycle = 0; dutyCycle < 64; dutyCycle++) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;
    case FSLOWC:
      // Soft start from stop or forward to slow reverse
      for (int dutyCycle = 64; dutyCycle > 0; dutyCycle--) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      digitalWrite(dirPin1, LOW); // Reverse direction
      digitalWrite(dirPin2, HIGH);
      for (int dutyCycle = 0; dutyCycle < 64; dutyCycle++) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;
    case FFASTC:
      // Soft start from stop or forward to slow reverse
      for (int dutyCycle = 150; dutyCycle > 0; dutyCycle--) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      digitalWrite(dirPin1, LOW); // Reverse direction
      digitalWrite(dirPin2, HIGH);
      for (int dutyCycle = 0; dutyCycle < 64; dutyCycle++) {
        analogWrite(pwmPin, dutyCycle);
        delay(10);
      }
      break;

    case RSLOWC:
      // Already moving slowly in reverse, no soft start needed
      analogWrite(pwmPin, 64);
      break;
  }
}

void sendStopcStatus() {
  StaticJsonDocument<256> responseDoc;
  responseDoc["client_type"] = "CCP";
  responseDoc["message"] = "STAT";
  responseDoc["client_id"] = "BR14"; // Replace with actual client ID if available
  responseDoc["sequence_number"] = String(strqSequenceNumber).c_str();
  responseDoc["status"] = "STOPC";

  char responseBuffer[256];
  size_t responseSize = serializeJson(responseDoc, responseBuffer);

  udp.beginPacket(server_ip, server_port);
  udp.write((const uint8_t*)responseBuffer, responseSize); // Cast to const uint8_t*
  udp.endPacket();

  // Increment the sequence number for STRQ messages
  strqSequenceNumber++;
}

void checkEmergencyBraking()  {
  int distance = 0;
  // Check the appropriate sensor based on CCP status
  switch (ccpStatus) {
    case FSLOWC:
      distance = S1_sonar.ping_cm();
      break;
    case FFASTC:
      // Moving forward, check the front sensor
      distance = S1_sonar.ping_cm();
      break;
    case RSLOWC:
      // Moving backward, check the rear sensor
      distance = S2_sonar.ping_cm();
      break;
    default:
      break;
  }

  // If distance is between 1 and 5 cm, trigger an emergency stop
  if (distance >= minBrakingDist && distance <= maxBrakingDist) {
    stopBladeRunner();
    handleLEDs(ledPins[3]);
    sendStopcStatus(); // Send STOPC status to CCP
  }
}

void checkIRSensor() {
  int lightValue;
  switch (ccpStatus) {
    case FSLOWC:
      lightValue = analogRead(IRSensorPin);
      
      if(lightValue > IRThreshold) {
        stopBladeRunner();
        handleLEDs(ledPins[2]);
        sendStopcStatus(); // Send STOPC status to CCP
      }
      break;
    case RSLOWC:
      lightValue = analogRead(IRSensorPin);
      
      if(lightValue > IRThreshold) {
        stopBladeRunner();
        handleLEDs(ledPins[2]);
        sendStopcStatus(); // Send STOPC status to CCP
      }
      break;
    default:
      break;
  }
}

void handleCommand(const char* action) {
  ccpStatus = getStatusFromString(action);
  switch(ccpStatus) {
    case STOPC:
      stopBladeRunner();
      handleLEDs(ledPins[2]);
      break;
    case STOPO:
      stopBladeRunner();
      handleLEDs(ledPins[2]);
      break;
    case FSLOWC:
      goForwardSlow();
      handleLEDs(ledPins[1]);
      break;
    case FFASTC:
      goForwardFast();
      handleLEDs(ledPins[0]);
      break;
    case RSLOWC:
      reverseSlow();
      handleLEDs(ledPins[1]);
      break;
    case OFLN:
      stopBladeRunner();
      handleLEDs(ledPins[2]);
      disconnectFun();
      break;
    default:
      return;
  }
}

unsigned long previousMillis = 0;
const long interval = 500;

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    checkEmergencyBraking();
    checkIRSensor();
  }

  // Check for incoming UDP packets
  int packetSize = udp.parsePacket();
  if (packetSize) {
    char incomingPacket[255];
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;
    }

    // Parse the received message
    StaticJsonDocument<256> doc;
    DeserializationError error = deserializeJson(doc, incomingPacket);
    if (error) {
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
      responseDoc["status"] = getStringFromStatus(ccpStatus);

      char responseBuffer[256];
      size_t responseSize = serializeJson(responseDoc, responseBuffer);

      udp.beginPacket(server_ip, server_port);
      udp.write((const uint8_t*)responseBuffer, responseSize); // Cast to const uint8_t*
      udp.endPacket();

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

      // Increment the sequence number for EXEC messages
      execSequenceNumber++;
    }
    // Print the most recent packet
  } 
}