#include <CheapStepper.h>
#include <Servo.h>

Servo servo1;

// Pin Definitions
#define IR_PIN 5
#define PROXI_PIN 6
#define BUZZER 12
int potPin = A0; // Analog input for soil/moisture sensor

// Variables
int fsoil = 0;

// Stepper setup (IN1, IN2, IN3, IN4)
CheapStepper stepper(8, 9, 10, 11);

void setup() {
  Serial.begin(9600);
  
  pinMode(PROXI_PIN, INPUT);   // PNP sensor, no pull-up used
  pinMode(IR_PIN, INPUT);
  pinMode(BUZZER, OUTPUT);
  
  servo1.attach(7);
  stepper.setRpm(17);

  // Initialize servo position
  servo1.write(180);
  delay(1000);
  servo1.write(70);
  delay(1000);

  Serial.println("System Ready...");
}

void loop() {
  fsoil = 0;
  int proxiState = digitalRead(PROXI_PIN);
  int irState = digitalRead(IR_PIN);

  // --- Metal Detection via Proximity Sensor ---
  if (proxiState == HIGH) {  // PNP NO → LOW when metal detected
    Serial.println("Metal detected!");
    tone(BUZZER, 1000, 1000);

    stepper.moveDegreesCW(240);
    delay(1500);
    stepper.stop();
    
    servo1.write(180);   // Open lid
    delay(1000);
    servo1.write(70);    // Close lid
    delay(1000);
    
    stepper.moveDegreesCCW(240);
    delay(1500);
    stepper.stop();
  }

  // --- Waste Detection via IR Sensor ---
  else if (irState == LOW) {  // Object detected
    tone(BUZZER, 1000, 300);
    delay(500);

    // Read soil/moisture 3 times for accuracy
    int totalSoil = 0;
    for (int i = 0; i < 3; i++) {
      int soil = analogRead(potPin);
      soil = constrain(soil, 485, 1023);
      totalSoil += map(soil, 485, 1023, 100, 0);
      delay(75);
    }

    fsoil = totalSoil / 3;
    Serial.print("Moisture: ");
    Serial.print(fsoil);
    Serial.println("%");

    // --- Wet Waste ---
    if (fsoil > 20) {
      Serial.println("Wet Waste Detected");
      tone(BUZZER, 1200, 300);
      
      stepper.moveDegreesCW(120);
      delay(1000);
      stepper.stop();

      servo1.write(180);
      delay(1000);
      servo1.write(70);
      delay(1000);

      stepper.moveDegreesCCW(120);
      delay(1000);
      stepper.stop();
    }

    // --- Dry Waste ---
    else {
      Serial.println("Dry Waste Detected");
      tone(BUZZER, 800, 300);
      servo1.write(180);
      delay(1000);
      servo1.write(70);
      delay(1000);
    }
  }

  else {
    // No detection → keep system idle
    stepper.stop();
  }

  delay(200); // Small delay for stability
}
