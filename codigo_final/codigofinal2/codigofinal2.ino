#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <ESP32Servo.h>
#include <LiquidCrystal.h>
#include <math.h>

// ===================== PINOS I2C =====================

#define PIN_SDA       21
#define PIN_SCL       22

// ===================== PINOS SERVOS =====================

#define PIN_SELECTOR  4
#define PIN_SORTER    2

// ===================== POSICOES SELECTOR =====================

#define SELECTOR_RECEBE   153
#define SELECTOR_SENSOR   100
#define SELECTOR_FUNIL    54

// ===================== POSICOES SORTER =====================

#define SORTER_VERMELHO   3
#define SORTER_LARANJA    39
#define SORTER_AMARELO    77
#define SORTER_VERDE      115
#define SORTER_AZUL       150

// ===================== CONFIG =====================

#define NUM_CORES     5
#define LIMIAR_DIST   0.15f

// ===================== OBJETOS =====================

Servo selector;
Servo sorter;

// LCD 16x2 (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(16, 17, 18, 19, 23, 5);

Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

// ===================== CORES =====================

const char* nomesCores[NUM_CORES] = {
  "Laranja",
  "Verde",
  "Vermelho",
  "Amarelo",
  "Azul"
};

float ref[NUM_CORES][3] = {
  {0.7190f, 0.1716f, 0.1094f},  // Laranja
  {0.3251f, 0.4162f, 0.2587f},  // Verde
  {0.7197f, 0.1509f, 0.1294f},  // Vermelho
  {0.5384f, 0.3352f, 0.1263f},  // Amarelo
  {0.2506f, 0.3891f, 0.3603f},  // Azul
};

// ===================== CONTAGEM =====================

int contagem[NUM_CORES] = {0, 0, 0, 0, 0};
int totalProcessados    = 0;

// ============================================================

void lerNormalizado(float &nr, float &ng, float &nb) {

  uint16_t r, g, b, clear;

  tcs.getRawData(&r, &g, &b, &clear);

  float total = (float)(r + g + b);

  if (total == 0.0f) {
    nr = ng = nb = 0.0f;
    return;
  }

  nr = r / total;
  ng = g / total;
  nb = b / total;
}

// ============================================================

float distEuclidiana(float nr, float ng, float nb, int c) {

  float dr = nr - ref[c][0];
  float dg = ng - ref[c][1];
  float db = nb - ref[c][2];

  return sqrtf(dr * dr + dg * dg + db * db);
}

// ============================================================

int classificar(float &menorDist) {

  float nr, ng, nb;

  lerNormalizado(nr, ng, nb);

  Serial.print("RGB -> ");
  Serial.print(nr, 4);
  Serial.print("  ");
  Serial.print(ng, 4);
  Serial.print("  ");
  Serial.println(nb, 4);

  menorDist = 9999.0f;
  int melhor = -1;

  for (int c = 0; c < NUM_CORES; c++) {

    float d = distEuclidiana(nr, ng, nb, c);

    if (d < menorDist) {
      menorDist = d;
      melhor = c;
    }
  }

  if (menorDist > LIMIAR_DIST)
    return -1;

  return melhor;
}

// ============================================================

void moverSorter(int cor) {

  switch (cor) {

    case 2: // Vermelho
      sorter.write(SORTER_VERMELHO);
      break;

    case 0: // Laranja
      sorter.write(SORTER_LARANJA);
      break;

    case 3: // Amarelo
      sorter.write(SORTER_AMARELO);
      break;

    case 1: // Verde
      sorter.write(SORTER_VERDE);
      break;

    case 4: // Azul
      sorter.write(SORTER_AZUL);
      break;
  }

  delay(1000);
}

// ============================================================

void setup() {

  Serial.begin(115200);

  Wire.begin(PIN_SDA, PIN_SCL);

  // LCD
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  // Sensor
  if (!tcs.begin()) {

    Serial.println("ERRO: TCS34725 nao encontrado!");

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERRO SENSOR");

    while (1);
  }

  // Servos
  selector.attach(PIN_SELECTOR);
  sorter.attach(PIN_SORTER);

  selector.write(SELECTOR_RECEBE);
  sorter.write(SORTER_VERMELHO);

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistema OK");

  Serial.println("Sistema iniciado.");

  delay(1500);
}

// ============================================================

void loop() {

  float dist;

  // ==================================================
  // 1. RECEBE MM
  // ==================================================

  selector.write(SELECTOR_RECEBE);

  Serial.println("Recebendo MM...");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Aguardando MM...");

  delay(5000);

  // ==================================================
  // 2. VAI PARA O SENSOR
  // ==================================================

  selector.write(SELECTOR_SENSOR);

  Serial.println("Lendo cor...");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Lendo cor...");

  delay(5000);

  // ==================================================
  // 3. IDENTIFICA COR
  // ==================================================

  int cor = classificar(dist);

  lcd.clear();

  if (cor >= 0) {

    Serial.print("Cor detectada: ");
    Serial.print(nomesCores[cor]);
    Serial.print("   dist=");
    Serial.println(dist, 4);

    lcd.setCursor(0, 0);
    lcd.print("Cor: ");
    lcd.print(nomesCores[cor]);

    lcd.setCursor(0, 1);
    lcd.print("Dist: ");
    lcd.print(dist, 3);

    moverSorter(cor);

    // Atualiza contagem
    contagem[cor]++;
    totalProcessados++;

    // Exibe placar no Serial
    Serial.println("--- Contagem atual ---");
    for (int i = 0; i < NUM_CORES; i++) {
      Serial.print(nomesCores[i]);
      Serial.print(": ");
      Serial.println(contagem[i]);
    }
    Serial.print("Total: ");
    Serial.println(totalProcessados);
    Serial.println("----------------------");

    // Exibe placar no LCD por 2s
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(nomesCores[cor]);
    lcd.print(": ");
    lcd.print(contagem[cor]);
    lcd.setCursor(0, 1);
    lcd.print("Total: ");
    lcd.print(totalProcessados);
    delay(2000);

  } else {

    Serial.print("Cor desconhecida   dist=");
    Serial.println(dist, 4);

    lcd.setCursor(0, 0);
    lcd.print("Desconhecida");

    lcd.setCursor(0, 1);
    lcd.print("Dist: ");
    lcd.print(dist, 3);
  }

  // ==================================================
  // 4. LIBERA NO FUNIL
  // ==================================================

  selector.write(SELECTOR_FUNIL);

  Serial.println("Liberando no funil...");

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Liberando...");

  delay(7000);
}
