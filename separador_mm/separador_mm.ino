#include <Wire.h>
#include <Adafruit_TCS34725.h>
#include <math.h>

// ─── Configurações ────────────────────────────────────────────────────────────

#define PIN_SDA       21
#define PIN_SCL       22

#define NUM_CORES     5
#define NUM_AMOSTRAS  20
#define LIMIAR_DIST   0.15f   // acima disso = cor não reconhecida

const char* nomesCores[NUM_CORES] = {
  "Laranja", "Verde", "Vermelho", "Amarelo", "Azul"
};

// Referências de calibração: média normalizada [r, g, b] de cada cor
float ref[NUM_CORES][3];

Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

// ─── Funções auxiliares ───────────────────────────────────────────────────────

// Aguarda o usuário pressionar ENTER no Serial Monitor
void aguardarEnter() {
  while (Serial.available()) Serial.read();   // descarta lixo no buffer
  while (!Serial.available());               // bloqueia até receber algo
  while (Serial.available()) Serial.read();   // descarta o ENTER recebido
}

// Lê o sensor e retorna r, g, b normalizados (soma = 1)
// Normalizar cancela a variação de intensidade causada por distância/luz ambiente
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

// Distância euclidiana entre leitura normalizada e referência da cor c
float distEuclidiana(float nr, float ng, float nb, int c) {
  float dr = nr - ref[c][0];
  float dg = ng - ref[c][1];
  float db = nb - ref[c][2];
  return sqrtf(dr*dr + dg*dg + db*db);
}

// ─── Calibração ───────────────────────────────────────────────────────────────

void calibrar() {
  delay(500);
  while (Serial.available()) Serial.read();

  Serial.println(F("\n========== MODO CALIBRAÇÃO =========="));
  Serial.println(F("Para cada cor: posicione o M&M sob o sensor e pressione ENTER.\n"));

  for (int c = 0; c < NUM_CORES; c++) {
    Serial.print(F(">>> Posicione o M&M "));
    Serial.print(nomesCores[c]);
    Serial.println(F(" e pressione ENTER..."));
    aguardarEnter();

    Serial.println(F("    Aguardando 2s..."));
    delay(2000);  // <-- delay antes de medir
    Serial.println(F("    Medindo..."));

    float somaR = 0.0f, somaG = 0.0f, somaB = 0.0f;

    for (int s = 0; s < 10; s++) {  // <-- 10 medições
      float nr, ng, nb;
      lerNormalizado(nr, ng, nb);
      somaR += nr;
      somaG += ng;
      somaB += nb;
      delay(100);
    }

    ref[c][0] = somaR / 10.0f;
    ref[c][1] = somaG / 10.0f;
    ref[c][2] = somaB / 10.0f;

    Serial.print(F("    Referencia "));
    Serial.print(nomesCores[c]);
    Serial.print(F(":  r=")); Serial.print(ref[c][0], 4);
    Serial.print(F("  g=")); Serial.print(ref[c][1], 4);
    Serial.print(F("  b=")); Serial.println(ref[c][2], 4);
  }

  Serial.println(F("\n========== Calibracao concluida! =========="));
  Serial.println(F("Iniciando modo de producao...\n"));
}

// ─── Classificação ────────────────────────────────────────────────────────────

// Retorna índice da cor mais próxima (ou -1 se distância > LIMIAR_DIST)
// menorDist recebe o valor da distância para fins de debug
int classificar(float &menorDist) {
  float nr, ng, nb;
  lerNormalizado(nr, ng, nb);

  menorDist = 9999.0f;
  int melhor = -1;

  for (int c = 0; c < NUM_CORES; c++) {
    float d = distEuclidiana(nr, ng, nb, c);
    if (d < menorDist) {
      menorDist = d;
      melhor    = c;
    }
  }

  if (menorDist > LIMIAR_DIST) return -1;
  return melhor;
}

// ─── Setup e Loop ─────────────────────────────────────────────────────────────

void setup() {
  Serial.begin(115200);
  while (!Serial);  // aguarda porta serial (necessário em algumas placas)

  Wire.begin(PIN_SDA, PIN_SCL);

  if (!tcs.begin()) {
    Serial.println(F("ERRO: TCS34725 nao encontrado! Verifique SDA/SCL e VCC."));
    while (1);
  }
  Serial.println(F("TCS34725 OK."));

  calibrar();
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
