#include <ESP32Servo.h>
#include "DHT.h"

// ==========================================
// PIN DEFINITIONS
// ==========================================
#define SERVO_PIN      18  // Servo PWM Signal
#define DHTPIN         16  // DHT11 Data Pin
#define DHTTYPE        DHT11

#define LDR_LEFT_PIN   4  // Left LDR (ADC1_CH6)
#define LDR_RIGHT_PIN  35  // Right LDR (ADC1_CH7)
#define SOLAR_PIN      32  // Solar Panel Voltage Divider (ADC1_CH4)

// ==========================================
// TRACKING CONFIGURATION
// ==========================================
#define LIGHT_THRESHOLD  150  // Difference required to move motor
#define SERVO_STEP       2    // Degrees per step
#define SERVO_MIN_ANGLE  20   // Min angle limit
#define SERVO_MAX_ANGLE  200  // Max angle limit
#define DHT_READ_INTERVAL 2500 // Read DHT11 every 2.5 seconds

Servo trackerServo;
DHT dht(DHTPIN, DHTTYPE, 15);

int currentAngle = 90; // Start at center position
unsigned long lastDHTRead = 0;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Attach Servo with pulse boundaries
  trackerServo.attach(SERVO_PIN, 1000, 2000);
  trackerServo.write(currentAngle);

  // Initialize Sensors
  dht.begin();
  pinMode(LDR_LEFT_PIN, INPUT);
  pinMode(LDR_RIGHT_PIN, INPUT);
  pinMode(SOLAR_PIN, INPUT);

  Serial.println("==================================================");
  Serial.println("  ESP32 Autonomous Solar Tracker System Online    ");
  Serial.println("==================================================");
}

void loop() {
  // 1. READ LIGHT LEVELS & TRACK SUN
  int leftLDR = analogRead(LDR_LEFT_PIN);
  int rightLDR = analogRead(LDR_RIGHT_PIN);
  int lightDiff = leftLDR - rightLDR;

  if (abs(lightDiff) > LIGHT_THRESHOLD) {
    if (lightDiff > 0 && currentAngle > SERVO_MIN_ANGLE) {
      currentAngle -= SERVO_STEP; // Rotate Left
    } 
    else if (lightDiff < 0 && currentAngle < SERVO_MAX_ANGLE) {
      currentAngle += SERVO_STEP; // Rotate Right
    }
    trackerServo.write(currentAngle);
  }

  // 2. READ SOLAR PANEL VOLTAGE
  int solarRaw = analogRead(SOLAR_PIN);
  float solarVolts = ((solarRaw / 4095.0) * 3.3) * 2.0; // Account for 10k/10k divider

  // 3. PERIODIC CLIMATE READINGS (DHT11)
  if (millis() - lastDHTRead > DHT_READ_INTERVAL) {
    lastDHTRead = millis();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    Serial.print("Angle: ");
    Serial.print(currentAngle);
    Serial.print("° | Left LDR: ");
    Serial.print(leftLDR);
    Serial.print(" | Right LDR: ");
    Serial.print(rightLDR);
    Serial.print(" | Solar: ");
    Serial.print(solarVolts, 2);
    Serial.print(" V | Temp: ");
    if (!isnan(t)) Serial.print(t, 1); else Serial.print("ERR");
    Serial.print("°C | Humidity: ");
    if (!isnan(h)) Serial.print(h, 1); else Serial.print("ERR");
    Serial.println("%");
  }

  delay(40); // Smooth tracking delay
}
