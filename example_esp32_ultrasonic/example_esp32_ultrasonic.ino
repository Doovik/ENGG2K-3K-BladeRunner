#include <NewPing.h>

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

  delay(10);  // Wait before starting the next loop
}
