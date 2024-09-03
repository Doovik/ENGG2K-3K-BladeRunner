#include <NewPing.h>
#include <Arduino.h>

#define S1_TRIGGER_PIN 25
#define S1_ECHO_PIN 26
#define S2_TRIGGER_PIN 27
#define S2_ECHO_PIN 14
#define S1_RED_PIN 16
#define S1_GREEN_PIN 17
#define S2_RED_PIN 18
#define S2_GREEN_PIN 19
#define MIN_LENGTH 10


#define maxDist 450

NewPing S1_sonar(S1_TRIGGER_PIN, S1_ECHO_PIN, maxDist);
NewPing S2_sonar(S2_TRIGGER_PIN, S2_ECHO_PIN, maxDist);

void setup()
{
  Serial.begin(9600);
  pinMode(S1_RED_PIN, OUTPUT);
  pinMode(S1_GREEN_PIN, OUTPUT);
  pinMode(S2_RED_PIN, OUTPUT);
  pinMode(S2_GREEN_PIN, OUTPUT);
}

void loop()
{
  int S1_distance = S1_sonar.ping_cm();
  delay(10);  // Wait for the first echo to dissipate
  int S2_distance = S2_sonar.ping_cm();
  if(S1_distance < 1) {
    digitalWrite(S1_RED_PIN,HIGH);
    Serial.println("S1 Unable to read");
  } else if(S1_distance < MIN_LENGTH) {
    Serial.print(": ");
    Serial.print(S2_distance);
    Serial.println("cm");
    digitalWrite(S1_RED_PIN,HIGH);
    digitalWrite(S1_GREEN_PIN,LOW);
  } else {
    Serial.print("S1: ");
    Serial.print(S1_distance);
    Serial.println("cm");
    digitalWrite(S1_RED_PIN,LOW);
    digitalWrite(S1_GREEN_PIN,HIGH);
  }

  if(S2_distance < 1) {
    digitalWrite(S2_RED_PIN,HIGH);
    Serial.println("S2 Unable to read");
  } else if(S2_distance < MIN_LENGTH) {
    Serial.print("S2: ");
    Serial.print(S2_distance);
    Serial.println("cm");
    digitalWrite(S2_RED_PIN,HIGH);
    digitalWrite(S2_GREEN_PIN,LOW);
  } else {
    Serial.print("S2: ");
    Serial.print(S2_distance);
    Serial.println("cm");
    digitalWrite(S2_RED_PIN,LOW);
    digitalWrite(S2_GREEN_PIN,HIGH);
  }

  delay(10);  // Wait before starting the next loop
}
