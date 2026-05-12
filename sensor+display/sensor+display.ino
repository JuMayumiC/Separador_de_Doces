#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <LiquidCrystal.h>
#include <math.h>

// -----------------------------
// PINOS
// -----------------------------
#define PIN_SDA       21
#define PIN_SCL       22

// LCD 16x2 (RS, E, D4, D5, D6, D7)
LiquidCrystal lcd(16, 17, 18, 19, 23, 5);

// -----------------------------
// CONFIGURAÇÕES
// -----------------------------
#define NUM_CORES     5
#define LIMIAR_DIST   0.15f

const char* nomesCores[NUM_CORES] = {
  "Laranja",
  "Verde",
  "Vermelho",
  "Amarelo",
  "Azul"
};

// referências calibradas
float ref[NUM_CORES][3] = {
  {0.7349f, 0.1525f, 0.1126f},  // Laranja
  {0.4025f, 0.4184f, 0.1791f},  // Verde
  {0.7381f, 0.1347f, 0.1272f},  // Vermelho
  {0.5909f, 0.2878f, 0.1213f},  // Amarelo
  {0.2721f, 0.3923f, 0.3356f},  // Azul
};

// -----------------------------
// SENSOR
// -----------------------------
Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

// -----------------------------
// LEITURA NORMALIZADA
// -----------------------------
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

// -----------------------------
// DISTÂNCIA EUCLIDIANA
// -----------------------------
float distEuclidiana(float nr, float ng, float nb, int c) {

  float dr = nr - ref[c][0];
  float dg = ng - ref[c][1];
  float db = nb - ref[c][2];

  return sqrtf(dr * dr + dg * dg + db * db);
}

// -----------------------------
// CLASSIFICAÇÃO
// -----------------------------
int classificar(float &menorDist) {

  float nr, ng, nb;

  lerNormalizado(nr, ng, nb);

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

// -----------------------------
// SETUP
// -----------------------------
void setup() {

  Wire.begin(PIN_SDA, PIN_SCL);

  // LCD
  lcd.begin(16, 2);

  lcd.setCursor(0, 0);
  lcd.print("Iniciando...");

  // Sensor
  if (!tcs.begin()) {

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("ERRO SENSOR");

    while (1);
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Sistema OK");
  delay(1500);
}

// -----------------------------
// LOOP
// -----------------------------
void loop() {

  float dist;

  int cor = classificar(dist);

  lcd.clear();

  if (cor >= 0) {

    lcd.setCursor(0, 0);
    lcd.print("Cor:");

    lcd.setCursor(5, 0);
    lcd.print(nomesCores[cor]);

    lcd.setCursor(0, 1);
    lcd.print("Dist:");

    lcd.setCursor(6, 1);
    lcd.print(dist, 3);

  } else {

    lcd.setCursor(0, 0);
    lcd.print("Desconhecida");

    lcd.setCursor(0, 1);
    lcd.print("Dist:");

    lcd.setCursor(6, 1);
    lcd.print(dist, 3);
  }

  delay(500);
}