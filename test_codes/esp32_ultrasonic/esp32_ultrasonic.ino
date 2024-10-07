#include <NewPing.h>
#include <Arduino.h>

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
NewPing S1_sonar(S1_TRIGGER_PIN, S1_ECHO_PIN, maxDist);
NewPing S2_sonar(S2_TRIGGER_PIN, S2_ECHO_PIN, maxDist);

void setup()
{
  Serial.begin(9600);
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);
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

void goReverseSlow() {
  digitalWrite(dirPin1, LOW); 
  digitalWrite(dirPin2, HIGH);    
  analogWrite(pwmPin, 64);        
  Serial.println("Moving slowly backward.");
}

bool checkEmergencyBraking()  {
  int distance = 0;
  if(digitalRead(dirPin1) == HIGH) {
    distance = S1_sonar.ping_cm();
    Serial.print("Front sensor distance: ");
  } else {
    distance = S2_sonar.ping_cm();
    Serial.print("Rear sensor distance: ");
  }

  Serial.println(distance);

  // If distance is between 1 and 5 cm, trigger an emergency stop
  if (distance >= minBrakingDist && distance <= maxBrakingDist) {
    stopBladeRunner();
    Serial.println("Emergency Stop: Object detected within 1-5 cm.");
    return true;
  }
  return false;
}

void loop() {
  if(checkEmergencyBraking()) {
    return;
  } else {
    goForwardSlow();
  }
  delay(100);
}