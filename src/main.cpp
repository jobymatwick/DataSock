#include <Arduino.h>

#define LED_PIN 13

void setup() {
    // initialize the digital pin as an output.
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    digitalWrite(LED_PIN, HIGH);
    delay(100);
    digitalWrite(LED_PIN, LOW);
    delay(100);
}