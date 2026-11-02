/*
 * Cabra Cabriola - articulacoes
 * 5x MG996R + 1x SG90 (boca) + 2x passo (L298N)
 *
 * Biblioteca: "ESP32Servo" (Gerenciador de Bibliotecas)
 * Stepper: usa Stepper.h (vem com Arduino)
 *
 * ========== ALIMENTACAO (CRITICO) ==========
 * Fonte externa 5V~6V, idealmente >= 5A (melhor 8~10A).
 *   Fonte +  ---- vermelho de TODOS os servos
 *   Fonte +  ---- VCC dos 2 L298N (motor)
 *   Fonte -  ---- marrom/GND de TODOS os servos
 *   Fonte -  ---- GND dos L298N
 *   Fonte -  ---- GND do ESP32   <<< GND COMUM OBRIGATORIO
 *
 * ESP32 alimentado por USB (so logica).
 * NUNCA alimentar MG996R pelo 5V do ESP32.
 *
 * Capacitor 1000uF no barramento dos servos (+ e -).
 *
 * ========== SERVOS -> GPIO (so o fio de sinal) ==========
 * MG996R / SG90: marrom=GND, vermelho=5~6V, laranja/amarelo=SINAL
 *
 *   Boca (SG90 9g)     -> GPIO 13
 *   Articulacao 1      -> GPIO 14
 *   Articulacao 2      -> GPIO 27
 *   Articulacao 3      -> GPIO 26
 *   Articulacao 4      -> GPIO 25
 *   Articulacao 5      -> GPIO 33
 *
 * ========== L298N PE ESQUERDO ==========
 * Jumpers ENA e ENB COLOCADOS.
 * Se motor <= 12V, jumper 5V-EN pode ficar.
 *
 *   IN1 -> GPIO 18
 *   IN2 -> GPIO 19
 *   IN3 -> GPIO 21
 *   IN4 -> GPIO 22
 *   OUT1..OUT4 -> bobinas do passo (A+, A-, B+, B-)
 *   GND  -> GND comum
 *   VCC  -> fonte dos motores
 *
 * ========== L298N PE DIREITO ==========
 *   IN1 -> GPIO 23
 *   IN2 -> GPIO 5
 *   IN3 -> GPIO 17
 *   IN4 -> GPIO 16
 *   OUT1..OUT4 -> bobinas do outro passo
 *   GND / VCC iguais
 *
 * Se o passo girar ao contrario: troque A+ com A- (ou inverta no codigo).
 * Se nao girar / vibrar: troque a ordem das bobinas (pares A e B).
 */

#include <ESP32Servo.h>
#include <Stepper.h>

// --- Servos ---
static const int PIN_BOCA = 13;
static const int PIN_ART[5] = {14, 27, 26, 25, 33};

Servo boca;
Servo arts[5];

// Angulos iniciais (ajuste depois na estrutura real)
static const int BOCA_FECHADA = 90;
static const int BOCA_ABERTA = 50;
static const int ART_CENTRO = 90;

// --- Passos (L298N) ---
// Ajuste STEPS_POR_REV conforme o motor (ex.: 2048 para 28BYJ-48 com reducao,
// 200 para NEMA17 1.8° sem micropasso)
static const int STEPS_POR_REV = 200;
static const int PASSO_RPM = 30;

Stepper peEsq(STEPS_POR_REV, 18, 19, 21, 22);
Stepper peDir(STEPS_POR_REV, 23, 5, 17, 16);

void attachServos() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  boca.setPeriodHertz(50);
  boca.attach(PIN_BOCA, 500, 2400);
  boca.write(BOCA_FECHADA);

  for (int i = 0; i < 5; i++) {
    arts[i].setPeriodHertz(50);
    arts[i].attach(PIN_ART[i], 500, 2400);
    arts[i].write(ART_CENTRO);
    delay(50);
  }
}

void abrirBoca() { boca.write(BOCA_ABERTA); }
void fecharBoca() { boca.write(BOCA_FECHADA); }

void moverArticulacao(int idx, int angulo) {
  if (idx < 0 || idx > 4) return;
  angulo = constrain(angulo, 0, 180);
  arts[idx].write(angulo);
}

void cabriolaDemo() {
  // Sequencia teatral simples: agacha -> estica -> boca
  Serial.println("Cabriola demo");
  for (int i = 0; i < 5; i++) arts[i].write(70);
  delay(400);
  for (int i = 0; i < 5; i++) arts[i].write(110);
  delay(400);
  abrirBoca();
  delay(400);
  fecharBoca();
  for (int i = 0; i < 5; i++) arts[i].write(ART_CENTRO);
}

void andarFrente(int passos) {
  // Ambos os pes no mesmo sentido (ajuste sinal se um motor for espelhado)
  peEsq.step(passos);
  peDir.step(passos);
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println();
  Serial.println("Cabra Cabriola - articulacoes");
  Serial.println("Comandos: b=boca | c=cabriola | f=frente | t=teste servos | 0-4=art");

  attachServos();
  peEsq.setSpeed(PASSO_RPM);
  peDir.setSpeed(PASSO_RPM);

  delay(500);
  Serial.println("Pronto.");
}

void loop() {
  if (!Serial.available()) {
    delay(10);
    return;
  }

  char cmd = Serial.read();
  switch (cmd) {
    case 'b':
    case 'B':
      abrirBoca();
      delay(500);
      fecharBoca();
      Serial.println("Boca ok");
      break;

    case 'c':
    case 'C':
      cabriolaDemo();
      break;

    case 'f':
    case 'F':
      Serial.println("Andar frente");
      andarFrente(STEPS_POR_REV / 4);  // 1/4 volta
      break;

    case 't':
    case 'T':
      Serial.println("Teste servos 0..180..90");
      for (int a = 0; a <= 180; a += 30) {
        boca.write(a);
        for (int i = 0; i < 5; i++) arts[i].write(a);
        delay(300);
      }
      boca.write(BOCA_FECHADA);
      for (int i = 0; i < 5; i++) arts[i].write(ART_CENTRO);
      break;

    case '0':
    case '1':
    case '2':
    case '3':
    case '4': {
      int idx = cmd - '0';
      Serial.printf("Art %d: 60 -> 120 -> 90\n", idx);
      moverArticulacao(idx, 60);
      delay(400);
      moverArticulacao(idx, 120);
      delay(400);
      moverArticulacao(idx, 90);
      break;
    }

    default:
      break;
  }
}
