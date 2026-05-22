#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <ESP32Servo.h>
#include <math.h>

#define PIN_SDA       21
#define PIN_SCL       22

#define PIN_SELECTOR  4
#define PIN_SORTER    2

#define NUM_CORES     5
#define LIMIAR_DIST   0.15f

// ===================== POSICOES SELECTOR =====================

#define SELECTOR_RECEBE   95
#define SELECTOR_SENSOR   45
#define SELECTOR_FUNIL    3

// ===================== POSICOES SORTER =====================

#define SORTER_VERMELHO   3
#define SORTER_LARANJA    39
#define SORTER_AMARELO    77
#define SORTER_VERDE      115
#define SORTER_AZUL       150

// ============================================================

Servo selector;
Servo sorter;

// ===================== CORES =====================

const char* nomesCores[NUM_CORES] = {
  "Laranja",
  "Verde",
  "Vermelho",
  "Amarelo",
  "Azul"
};

float ref[NUM_CORES][3] = {

  {0.7349f, 0.1525f, 0.1126f},  // Laranja
  {0.4025f, 0.4184f, 0.1791f},  // Verde
  {0.7381f, 0.1347f, 0.1272f},  // Vermelho
  {0.5909f, 0.2878f, 0.1213f},  // Amarelo
  {0.2721f, 0.3923f, 0.3356f},  // Azul

};

// ============================================================

Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

// ============================================================

void lerNormalizado(float &nr, float &ng, float &nb) {

  uint16_t r, g, b, clear;

  tcs.getRawData(&r, &g, &b, &clear);

  float total = (float)(r + g + b);

  if (total == 0.0f) {

    nr = 0.0f;
    ng = 0.0f;
    nb = 0.0f;

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

  if (menorDist > LIMIAR_DIST) {
    return -1;
  }

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

  // Inicializa sensor
  if (!tcs.begin()) {

    Serial.println("ERRO: TCS34725 nao encontrado!");

    while (1);
  }

  // Inicializa servos
  selector.attach(PIN_SELECTOR);
  sorter.attach(PIN_SORTER);

  // Posicao inicial
  selector.write(SELECTOR_RECEBE);
  sorter.write(SORTER_VERMELHO);

  Serial.println("Sistema iniciado.");
}

// ============================================================

void loop() {

  float dist;

  // ==================================================
  // 1. RECEBE MM
  // ==================================================

  selector.write(SELECTOR_RECEBE);

  Serial.println("Recebendo MM...");

  // fica 5 segundos em 103°
  delay(5000);

  // ==================================================
  // 2. VAI PARA O SENSOR
  // ==================================================

  selector.write(SELECTOR_SENSOR);

  Serial.println("Lendo cor...");

  // fica 5 segundos em 40°
  delay(5000);

  // ==================================================
  // 3. IDENTIFICA COR
  // ==================================================

  int cor = classificar(dist);

  if (cor >= 0) {

    Serial.print("Cor detectada: ");
    Serial.print(nomesCores[cor]);

    Serial.print("   dist=");
    Serial.println(dist, 4);

    moverSorter(cor);

  } else {

    Serial.print("Cor desconhecida   dist=");
    Serial.println(dist, 4);
  }

  // ==================================================
  // 4. LIBERA NO FUNIL
  // ==================================================

  selector.write(SELECTOR_FUNIL);

  Serial.println("Liberando no funil...");

  // fica 7 segundos em 3°
  delay(7000);
}