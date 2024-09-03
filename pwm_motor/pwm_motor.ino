// Define pins
const int pwmPin = 33;   // PWM output pin
const int dirPin1 = 2;   // Direction control pin 1
const int dirPin2 = 15;  // Direction control pin 2



void setup() {
  pinMode(dirPin1, OUTPUT);
  pinMode(dirPin2, OUTPUT);
  pinMode(pwmPin, OUTPUT);
}

void loop() {
  digitalWrite(dirPin1, HIGH);
  digitalWrite(dirPin2, LOW);
  analogWrite(pwmPin, 128); 

  delay(5000);

  analogWrite(pwmPin, 0);

  delay(1000);

  digitalWrite(dirPin1, LOW);
  digitalWrite(dirPin2, HIGH);
  analogWrite(pwmPin, 128); 

  delay(5000);

  analogWrite(pwmPin, 0);

  delay(1000);

}
