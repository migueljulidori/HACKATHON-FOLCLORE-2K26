/*
 * Cabra Cabriola - DFPlayer Mini + ESP32 (setup que mais funciona na pratica)
 *
 * Fontes: wiki DFRobot DFR0299, GetStarted oficial, issues da lib (ACK=false
 * em clones), ProtoSupplies/foruns (cap no VCC, speaker ligado, SD presente).
 *
 * ---------- HARDWARE (ordem de importancia) ----------
 * 1) DFPlayer VCC no 5V/VIN do ESP (USB). NUNCA no 3V3.
 * 2) Ligar os DOIS GND do DFPlayer no GND do ESP (pinos 7 e 10 do modulo).
 * 3) Capacitor 470uF~1000uF: + no VCC do DFPlayer, - no GND (bem perto).
 * 4) Speaker em SPK_1 e SPK_2 ANTES de ligar (sem speaker, ACK/begin falha
 *    em varios clones).
 * 5) SD FAT32 inserido ANTES de ligar. Sem SD, UART costuma falhar.
 * 6) UART:
 *      ESP32 GPIO32 (RX) <- DFPlayer TX
 *      ESP32 GPIO33 (TX) -> DFPlayer RX
 *    1k no TX->RX e opcional no ESP32 (3.3V). Sem 1k ok.
 * 7) Botao teste: GPIO26 -> botao -> GND
 *
 * ---------- SD ----------
 * Formatar FAT32. Copiar UM arquivo por vez, nesta ordem:
 *   primeiro:  mp3/0001.mp3   (ou 0001.mp3 na raiz)
 * Pasta "mp3" em minusculas. Sem lixeira/.Trash no cartao.
 *
 * ---------- CODIGO ----------
 * Clones (MH2024K/GD3200): begin(..., isACK=false) e timeout maior.
 * play(1) toca o 1o MP3 na ORDEM DE COPIA do cartao.
 */

#include "Arduino.h"
#include "DFRobotDFPlayerMini.h"

static const int PIN_DF_RX = 32;  // <- DFPlayer TX
static const int PIN_DF_TX = 33;  // -> DFPlayer RX
static const int PIN_BOTAO = 26;
static const uint8_t VOLUME = 30;  // 0..30; baixo evita "teck" por queda de tensao

HardwareSerial dfSerial(2);
DFRobotDFPlayerMini dfplayer;
bool dfOk = false;

bool iniciarDFPlayer() {
  // Metodo que mais resolve em clone: SEM exigir ACK
  Serial.println("begin(isACK=false) ...");
  if (dfplayer.begin(dfSerial, /*isACK=*/false, /*doReset=*/true)) {
    dfplayer.setTimeOut(1000);  // MH2024K/GD3200 sao lentos
    return true;
  }

  delay(1000);

  // Fallback com ACK (modulo original DFRobot)
  Serial.println("begin(isACK=true) ...");
  if (dfplayer.begin(dfSerial, /*isACK=*/true, /*doReset=*/true)) {
    dfplayer.setTimeOut(500);
    return true;
  }

  return false;
}

void tocar() {
  if (!dfOk) return;

  dfplayer.volume(VOLUME);
  delay(100);

  // 1) Ordem de copia no SD (mais compativel entre chips)
  Serial.println("play(1)");
  dfplayer.play(1);
  delay(200);

  // 2) Reforco: pasta mp3/0001.mp3
  Serial.println("playMp3Folder(1)");
  dfplayer.playMp3Folder(1);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println();
  Serial.println("Cabra Cabriola / DFPlayer - setup recomendado");
  Serial.println("Confirme: 5V, 2xGND, cap, speaker SPK, SD no slot");

  pinMode(PIN_BOTAO, INPUT_PULLUP);

  dfSerial.begin(9600, SERIAL_8N1, PIN_DF_RX, PIN_DF_TX);

  // Wiki: inicializacao pode levar 3~5 s (SD + chip)
  Serial.println("Aguardando 4s (boot do modulo + SD)...");
  delay(4000);

  if (!iniciarDFPlayer()) {
    Serial.println("Falha begin.");
    Serial.println("- VCC=5V? Speaker em SPK1/SPK2?");
    Serial.println("- SD encaixado ANTES de ligar?");
    Serial.println("- Os 2 GND do DFPlayer no GND do ESP?");
    dfOk = false;
    return;
  }

  dfOk = true;
  Serial.println("DFPlayer online");

  delay(300);
  dfplayer.volume(VOLUME);
  delay(100);
  dfplayer.EQ(DFPLAYER_EQ_NORMAL);
  delay(100);
  dfplayer.outputDevice(DFPLAYER_DEVICE_SD);
  delay(300);

  tocar();
}

void loop() {
  static bool botaoAntes = HIGH;
  static unsigned long ultimo = 0;

  bool botao = digitalRead(PIN_BOTAO);
  if (botaoAntes == HIGH && botao == LOW && millis() - ultimo > 1000) {
    ultimo = millis();
    Serial.println("--- botao ---");
    tocar();
  }
  botaoAntes = botao;

  if (!dfOk) return;

  if (dfplayer.available()) {
    uint8_t tipo = dfplayer.readType();
    int valor = dfplayer.read();

    if (tipo == DFPlayerPlayFinished) {
      Serial.printf("OK terminou faixa %d\n", valor);
    } else if (tipo == DFPlayerError) {
      Serial.printf("Erro %d ", valor);
      if (valor == 5 || valor == 6) Serial.println("(arquivo nao encontrado)");
      else if (valor == 8) Serial.println("(SD)");
      else Serial.println();
    }
  }
}
