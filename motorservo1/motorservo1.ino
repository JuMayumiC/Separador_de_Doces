#include <ESP32Servo.h>

Servo selector;
Servo sorter;

int currentSelector = 90;
int currentSorter = 90;

void setup() {
  Serial.begin(9600);

  // SELECTOR no GPIO 4
  selector.attach(4);

  // SORTER no GPIO 21
  sorter.attach(21);

  selector.write(currentSelector);
  sorter.write(currentSorter);

  delay(1000);

  Serial.println("=== CALIBRACAO DE SERVOS ===");
  Serial.println("Comandos:");
  Serial.println("S angulo  -> mover SELECTOR");
  Serial.println("T angulo  -> mover SORTER");
  Serial.println("Exemplo: S 120");
  Serial.println("---------------------------");
}

void loop() {

  if (Serial.available()) {

    char servo = Serial.read();

    int angle = Serial.parseInt();

    if (servo == 'S') {

      currentSelector = constrain(angle, 0, 180);

      selector.write(currentSelector);

      Serial.print("Selector -> ");
      Serial.println(currentSelector);
    }

    else if (servo == 'T') {

      currentSorter = constrain(angle, 0, 180);

      sorter.write(currentSorter);

      Serial.print("Sorter -> ");
      Serial.println(currentSorter);
    }

    delay(200);
  }
}

