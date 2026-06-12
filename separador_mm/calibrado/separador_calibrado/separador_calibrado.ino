#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <math.h>

#define PIN_SDA       21
#define PIN_SCL       22

#define NUM_CORES     5
#define LIMIAR_DIST   0.15f

const char* nomesCores[NUM_CORES] = {
  "Laranja", "Verde", "Vermelho", "Amarelo", "Azul"
};

float ref[NUM_CORES][3] = {
  {0.7190f, 0.1716f, 0.1094f},  // Laranja
  {0.3251f, 0.4162f, 0.2587f},  // Verde
  {0.7197f, 0.1509f, 0.1294f},  // Vermelho
  {0.5384f, 0.3352f, 0.1263f},  // Amarelo
  {0.2506f, 0.3891f, 0.3603f},  // Azul
};

Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

void lerNormalizado(float &nr, float &ng, float &nb) {
  uint16_t r, g, b, clear;
  tcs.getRawData(&r, &g, &b, &clear);
  float total = (float)(r + g + b);
  if (total == 0.0f) { nr = ng = nb = 0.0f; return; }
  nr = r / total;
  ng = g / total;
  nb = b / total;
}

float distEuclidiana(float nr, float ng, float nb, int c) {
  float dr = nr - ref[c][0];
  float dg = ng - ref[c][1];
  float db = nb - ref[c][2];
  return sqrtf(dr*dr + dg*dg + db*db);
}

int classificar(float &menorDist) {
  float nr, ng, nb;
  lerNormalizado(nr, ng, nb);
  menorDist = 9999.0f;
  int melhor = -1;
  for (int c = 0; c < NUM_CORES; c++) {
    float d = distEuclidiana(nr, ng, nb, c);
    if (d < menorDist) { menorDist = d; melhor = c; }
  }
  if (menorDist > LIMIAR_DIST) return -1;
  return melhor;
}

void setup() {
  Serial.begin(115200);
  Wire.begin(PIN_SDA, PIN_SCL);
  if (!tcs.begin()) {
    Serial.println(F("ERRO: TCS34725 nao encontrado!"));
    while (1);
  }
  Serial.println(F("TCS34725 OK. Iniciando..."));
}

void loop() {
  float dist;
  int cor = classificar(dist);
  if (cor >= 0) {
    Serial.print(F("Cor: "));
    Serial.print(nomesCores[cor]);
    Serial.print(F("   dist="));
    Serial.println(dist, 4);
  } else {
    Serial.print(F("Desconhecida   dist="));
    Serial.println(dist, 4);
  }
  delay(500);
}