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

#define pwmPin 33
#define dirPin1 2
#define dirPin2 15

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
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(pwmPin, OUTPUT);
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);
}

void printSensor(String sensor, int distance, int onLED, int offLED) {
    Serial.print(sensor + ": ");
    if(distance < 1) {
      Serial.print("Unable to Read");
    } else {
      Serial.print(distance);
    }
    
    Serial.println("cm");
    digitalWrite(offLED, LOW);
    digitalWrite(onLED, HIGH);
}

void loop()
{
  int S1_distance = S1_sonar.ping_cm();
  delay(10);  // Wait for the first echo to dissipate
  int S2_distance = S2_sonar.ping_cm();

  if (S1_distance < 1) {
    printSensor("S1", S1_distance, S1_RED_PIN, S1_GREEN_PIN);
    analogWrite(pwmPin, 0);
  } else if (S1_distance <= MIN_LENGTH) {
    printSensor("S1", S1_distance, S1_RED_PIN, S1_GREEN_PIN);
    analogWrite(pwmPin, 0);
  } else if (S1_distance > MIN_LENGTH && S1_distance <= 10) {
    printSensor("S1", S1_distance, S1_GREEN_PIN, S1_RED_PIN);
    analogWrite(pwmPin, 64);  // Slow speed
  } else if (S1_distance > 10 && S1_distance <= 20) {
    printSensor("S1", S1_distance, S1_GREEN_PIN, S1_RED_PIN);
    analogWrite(pwmPin, 128);  // Medium speed
  } else {
    printSensor("S1", S1_distance, S1_GREEN_PIN, S1_RED_PIN);
    analogWrite(pwmPin, 200);  // Fast speed
  }

  if(S2_distance < 1) {
    printSensor("S2", S2_distance, S2_RED_PIN, S2_GREEN_PIN);
  } else if(S2_distance < MIN_LENGTH) {
   printSensor("S2", S2_distance, S2_RED_PIN, S2_GREEN_PIN);
  } else {
    printSensor("S2", S2_distance, S2_GREEN_PIN, S2_RED_PIN);
  }

  Serial.print(analogRead(pwmPin));
  
  delay(100);  // Wait before starting the next loop
}