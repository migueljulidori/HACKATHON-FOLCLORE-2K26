/*
 * ESP32 SoftAP + pagina web com joystick
 * 2 motores amarelos + 3x MG996R + LED (1 rele: NC=verde, NO=vermelho)
 *
 * WiFi "Robo-ESP32" / "12345678" -> http://192.168.4.1
 *
 * Motor ESQUERDO = modulo GPIO32/33 (motor 1)
 * Motor DIREITO  = modulo GPIO25/26 (motor 2)
 * Se lado invertido, troque os fios dos motores ou os pinos abaixo.
 *
 * LED painel (rele):
 *   1 rele (GPIO23): NC = verde (normal), NO = vermelho (bravo)
 *   IN1 -> GPIO23 | JD-VCC no 5V | GND comum
 *
 * Olhos (LEDs diretos no ESP, sem rele):
 *   Normal = brancos acesos, vermelhos apagados
 *   Bravo  = vermelhos acesos, brancos apagados
 *
 *   GPIO19 -> resistor 220Ω -> anodo LED branco ESQUERDO
 *   GPIO19 -> resistor 220Ω -> anodo LED branco DIREITO
 *   GPIO21 -> resistor 220Ω -> anodo LED vermelho ESQUERDO
 *   GPIO21 -> resistor 220Ω -> anodo LED vermelho DIREITO
 *   Catodos (todos) -> GND do ESP
 *
 *   (Os dois brancos no mesmo GPIO19; os dois vermelhos no GPIO21)
 *
 * HC-SR501 (presenca -> modo bravo automatico):
 *   VCC  -> 5V | GND -> GND | OUT -> GPIO18
 *   Aquecimento ~30–60 s. Jumper do modulo em H (retrigger).
 *   Potenciometros no modulo (visto por tras, lente pra voce):
 *     Tx (tempo)  -> gire NO SENTIDO ANTI-HORARIO = minimo (~3–5 s)
 *     Sx (sensib.)-> ajuste ate detectar bem sem falso alarme
 *   A demora para APAGAR vem quase sempre do pot Tx, nao do codigo.
 *   Presenca = Bravo. Sem OUT em HIGH = Normal (rapido no codigo).
 *
 * Joystick:
 *   frente  -> os dois frente
 *   tras    -> os dois tras
 *   direita -> esquerdo frente, direito tras
 *   esquerda-> esquerdo tras, direito frente
 *   centro  -> parado
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ESP32Servo.h>

const char *WIFI_SSID = "Robo-ESP32";
const char *WIFI_PASS = "12345678";

// Motor 1 = ESQUERDO | Motor 2 = DIREITO
const uint8_t pinosRele[2][2] = {
  {32, 33},  // esquerdo
  {25, 26}   // direito
};

const uint8_t PIN_LED_MODO = 23;     // rele painel: NC=verde, NO=vermelho
const uint8_t PIN_PIR = 18;          // HC-SR501 OUT
const uint8_t PIN_OLHO_BRANCO = 19;  // 2x LED branco (E+D)
const uint8_t PIN_OLHO_VERMELHO = 21; // 2x LED vermelho (E+D)

const uint8_t RELE_NO = LOW;
const uint8_t RELE_NC = HIGH;

// Liga bravo no primeiro HIGH. So atrasa o retorno a normal se OUT oscilar.
const unsigned long PIR_OFF_STABLE_MS = 150;

const int PIN_BOCA = 13;
// Trocados: E/D estavam invertidos na montagem
const int PIN_BRACO_E = 27;
const int PIN_BRACO_D = 14;

const int BOCA_FECHADA = 90;
const int BOCA_ABERTA = 50;
const int BRACO_CENTRO = 90;

WebServer server(80);
Servo servoBoca;
Servo servoBracoE;
Servo servoBracoD;

int angBoca = BOCA_FECHADA;
int angE = BRACO_CENTRO;
int angD = BRACO_CENTRO;
bool modoBravo = false;
bool presenca = false;
char dirAtual = 'S';
unsigned long pirLastHighMs = 0;
unsigned long pirLastDebugMs = 0;

void frente(uint8_t m) {
  digitalWrite(pinosRele[m][0], RELE_NC);
  digitalWrite(pinosRele[m][1], RELE_NC);
}

void tras(uint8_t m) {
  digitalWrite(pinosRele[m][0], RELE_NO);
  digitalWrite(pinosRele[m][1], RELE_NO);
}

void parado(uint8_t m) {
  digitalWrite(pinosRele[m][0], RELE_NO);
  digitalWrite(pinosRele[m][1], RELE_NC);
}

void setModo(bool bravo) {
  bool mudou = (modoBravo != bravo);
  modoBravo = bravo;
  // Rele painel: NC = verde normal | NO = vermelho bravo
  digitalWrite(PIN_LED_MODO, bravo ? RELE_NO : RELE_NC);
  // Olhos: normal = branco | bravo = vermelho
  digitalWrite(PIN_OLHO_BRANCO, bravo ? LOW : HIGH);
  digitalWrite(PIN_OLHO_VERMELHO, bravo ? HIGH : LOW);
  if (mudou) Serial.println(bravo ? "Modo BRAVO" : "Modo normal");
}

void atualizarPir() {
  bool raw = digitalRead(PIN_PIR) == HIGH;

  if (raw) {
    pirLastHighMs = millis();
    if (!presenca) {
      presenca = true;
      setModo(true);  // imediato ao detectar
      Serial.println("PIR: presenca -> BRAVO");
    }
  } else if (presenca && (millis() - pirLastHighMs) >= PIR_OFF_STABLE_MS) {
    presenca = false;
    setModo(false);
    Serial.println("PIR: livre -> normal");
  }

  if (millis() - pirLastDebugMs > 2000) {
    pirLastDebugMs = millis();
    Serial.printf("PIR raw=%d presenca=%d modo=%s\n",
                  raw ? 1 : 0, presenca ? 1 : 0, modoBravo ? "BRAVO" : "normal");
  }
}

void drive(char dir) {
  dirAtual = dir;
  switch (dir) {
    case 'F':  // ambos frente
      frente(0);
      frente(1);
      break;
    case 'T':  // ambos tras
      tras(0);
      tras(1);
      break;
    case 'R':  // direita: esq frente, dir tras
      frente(0);
      tras(1);
      break;
    case 'L':  // esquerda: esq tras, dir frente
      tras(0);
      frente(1);
      break;
    default:   // stop
      parado(0);
      parado(1);
      dirAtual = 'S';
      break;
  }
}

void setBoca(bool aberta) {
  angBoca = aberta ? BOCA_ABERTA : BOCA_FECHADA;
  servoBoca.write(angBoca);
}

void setBracoE(int a) {
  angE = constrain(a, 0, 180);
  servoBracoE.write(angE);
}

void setBracoD(int a) {
  angD = constrain(a, 0, 180);
  // Sentido mecanico invertido neste braco
  servoBracoD.write(180 - angD);
}

void attachServos() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  servoBoca.setPeriodHertz(50);
  servoBracoE.setPeriodHertz(50);
  servoBracoD.setPeriodHertz(50);
  servoBoca.attach(PIN_BOCA, 500, 2400);
  servoBracoE.attach(PIN_BRACO_E, 500, 2400);
  servoBracoD.attach(PIN_BRACO_D, 500, 2400);
  setBoca(false);
  setBracoE(BRACO_CENTRO);
  setBracoD(BRACO_CENTRO);
}

const char PAGE[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="pt-BR">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,maximum-scale=1,user-scalable=no">
<title>Robo ESP32</title>
<style>
  :root {
    --bg:#1a1d23; --card:#252a33; --text:#eef1f6; --muted:#9aa3b2;
    --accent:#3d9cfd; --go:#3ecf8e; --stop:#e85d5d; --bravo:#e53935; --normal:#43a047;
  }
  *{box-sizing:border-box;-webkit-tap-highlight-color:transparent;touch-action:none}
  body{
    margin:0;font-family:system-ui,sans-serif;background:var(--bg);color:var(--text);
    padding:16px;max-width:480px;margin-inline:auto;user-select:none;
  }
  h1{font-size:1.2rem;margin:0 0 4px}
  .sub{color:var(--muted);font-size:.85rem;margin-bottom:14px}
  section{background:var(--card);border-radius:14px;padding:14px;margin-bottom:12px}
  h2{font-size:.9rem;margin:0 0 10px;color:var(--muted);font-weight:600}
  .row{display:flex;gap:8px;flex-wrap:wrap}
  button{
    flex:1;min-width:70px;border:0;border-radius:12px;padding:14px 10px;
    font-size:1rem;font-weight:600;color:#fff;background:#3a4250;
  }
  button:active{opacity:.9;transform:scale(.97)}
  .normal{background:var(--normal)}
  .bravo{background:var(--bravo)}
  .accent{background:var(--accent)}
  .active{outline:2px solid #fff}
  #pad{
    width:220px;height:220px;margin:8px auto;border-radius:50%;
    background:#1e232c;border:3px solid #3a4250;position:relative;
    touch-action:none;
  }
  #knob{
    width:84px;height:84px;border-radius:50%;background:var(--accent);
    position:absolute;left:50%;top:50%;transform:translate(-50%,-50%);
    box-shadow:0 4px 12px rgba(0,0,0,.35);
  }
  #dirLabel{text-align:center;color:var(--muted);font-size:.85rem;margin-top:6px}
  .val{float:right;color:var(--text)}
  input[type=range]{width:100%}
  #status{font-size:.8rem;color:var(--muted);margin-top:8px}
</style>
</head>
<body>
  <h1>Controle do robo</h1>
  <p class="sub">Joystick + modos normal/bravo</p>

  <section>
    <h2>Modo (LED) <span id="pirTag" style="float:right;font-size:.8rem;color:#9aa3b2">PIR...</span></h2>
    <div class="row">
      <button id="btnN" class="normal active" onclick="setModo('N')">Normal (verde)</button>
      <button id="btnB" class="bravo" onclick="setModo('B')">Bravo (vermelho)</button>
    </div>
    <p style="margin:8px 0 0;font-size:.8rem;color:#9aa3b2">Presenca no HC-SR501 ativa Bravo sozinho</p>
  </section>

  <section>
    <h2>Locomocao</h2>
    <div id="pad"><div id="knob"></div></div>
    <div id="dirLabel">parado</div>
  </section>

  <section>
    <h2>Boca</h2>
    <div class="row">
      <button class="accent" onclick="cmd('/boca?a=1')">Abrir</button>
      <button onclick="cmd('/boca?a=0')">Fechar</button>
    </div>
  </section>

  <section>
    <h2>Braco esquerdo <span class="val" id="ve">90</span>°</h2>
    <input type="range" min="0" max="180" value="90" id="re"
      oninput="ve.textContent=this.value"
      onchange="cmd('/braco?lado=E&ang='+this.value)">
    <div class="row" style="margin-top:8px">
      <button onclick="setArm('E',0)">0</button>
      <button onclick="setArm('E',90)">90</button>
      <button onclick="setArm('E',180)">180</button>
    </div>
  </section>

  <section>
    <h2>Braco direito <span class="val" id="vd">90</span>°</h2>
    <input type="range" min="0" max="180" value="90" id="rd"
      oninput="vd.textContent=this.value"
      onchange="cmd('/braco?lado=D&ang='+this.value)">
    <div class="row" style="margin-top:8px">
      <button onclick="setArm('D',0)">0</button>
      <button onclick="setArm('D',90)">90</button>
      <button onclick="setArm('D',180)">180</button>
    </div>
  </section>

  <p id="status">ok</p>

<script>
const pad=document.getElementById('pad');
const knob=document.getElementById('knob');
const dirLabel=document.getElementById('dirLabel');
const nomes={F:'frente',T:'tras',L:'esquerda',R:'direita',S:'parado'};
let lastDir='S', sending=false;

async function cmd(url){
  const s=document.getElementById('status');
  try{ s.textContent=await (await fetch(url)).text(); }
  catch(e){ s.textContent='erro de rede'; }
}

function setModo(m){
  document.getElementById('btnN').classList.toggle('active', m==='N');
  document.getElementById('btnB').classList.toggle('active', m==='B');
  cmd('/modo?m='+m);
}

function setArm(lado,ang){
  const id=lado==='E'?'re':'rd';
  const vid=lado==='E'?'ve':'vd';
  document.getElementById(id).value=ang;
  document.getElementById(vid).textContent=ang;
  cmd('/braco?lado='+lado+'&ang='+ang);
}

async function sendDir(d){
  if(d===lastDir || sending) return;
  sending=true; lastDir=d;
  dirLabel.textContent=nomes[d]||d;
  try{ await fetch('/drive?d='+d); }catch(e){}
  sending=false;
}

function dirFromXY(x,y){
  const dead=0.28;
  const r=Math.hypot(x,y);
  if(r<dead) return 'S';
  if(Math.abs(y)>=Math.abs(x)) return y<0?'F':'T';
  return x>0?'R':'L';
}

function moveKnob(clientX, clientY){
  const r=pad.getBoundingClientRect();
  const cx=r.left+r.width/2, cy=r.top+r.height/2;
  let x=(clientX-cx)/(r.width/2);
  let y=(clientY-cy)/(r.height/2);
  const lim=0.85;
  const m=Math.hypot(x,y);
  if(m>lim){ x=x/m*lim; y=y/m*lim; }
  knob.style.left=((x+1)/2*100)+'%';
  knob.style.top=((y+1)/2*100)+'%';
  sendDir(dirFromXY(x,y));
}

function resetKnob(){
  knob.style.left='50%'; knob.style.top='50%';
  sendDir('S');
}

let active=false;
pad.addEventListener('pointerdown',e=>{
  active=true; pad.setPointerCapture(e.pointerId);
  moveKnob(e.clientX,e.clientY);
});
pad.addEventListener('pointermove',e=>{
  if(!active) return;
  moveKnob(e.clientX,e.clientY);
});
function endPtr(){ active=false; resetKnob(); }
pad.addEventListener('pointerup',endPtr);
pad.addEventListener('pointercancel',endPtr);

async function refreshStatus(){
  try{
    const j=await (await fetch('/status')).json();
    document.getElementById('btnN').classList.toggle('active', !j.bravo);
    document.getElementById('btnB').classList.toggle('active', !!j.bravo);
    document.getElementById('pirTag').textContent=j.pir?'Presenca':'Sem presenca';
    document.getElementById('pirTag').style.color=j.pir?'#e53935':'#9aa3b2';
  }catch(e){}
}
setInterval(refreshStatus, 800);
refreshStatus();
</script>
</body>
</html>
)HTML";

void handleRoot() {
  server.send_P(200, "text/html", PAGE);
}

void handleDrive() {
  String d = server.arg("d");
  d.toUpperCase();
  char c = d.length() ? d[0] : 'S';
  if (c != 'F' && c != 'T' && c != 'L' && c != 'R') c = 'S';
  drive(c);
  server.send(200, "text/plain", String("dir ") + c);
}

void handleModo() {
  String m = server.arg("m");
  m.toUpperCase();
  bool bravo = (m == "B" || m == "BRAVO" || m == "R");
  setModo(bravo);
  server.send(200, "text/plain", bravo ? "modo bravo (vermelho)" : "modo normal (verde)");
}

void handleStatus() {
  String j = "{";
  j += "\"bravo\":";
  j += modoBravo ? "true" : "false";
  j += ",\"pir\":";
  j += presenca ? "true" : "false";
  j += "}";
  server.send(200, "application/json", j);
}

void handleBoca() {
  bool aberta = server.arg("a").toInt() != 0;
  setBoca(aberta);
  server.send(200, "text/plain", aberta ? "boca aberta" : "boca fechada");
}

void handleBraco() {
  String lado = server.arg("lado");
  lado.toUpperCase();
  int ang = server.arg("ang").toInt();
  if (lado == "E") {
    setBracoE(ang);
    server.send(200, "text/plain", "braco E " + String(angE));
  } else if (lado == "D") {
    setBracoD(ang);
    server.send(200, "text/plain", "braco D " + String(angD));
  } else {
    server.send(400, "text/plain", "lado=E|D");
  }
}

void setupWifiPortal() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_SSID, WIFI_PASS);
  delay(200);

  server.on("/", handleRoot);
  server.on("/drive", handleDrive);
  server.on("/modo", handleModo);
  server.on("/boca", handleBoca);
  server.on("/braco", handleBraco);
  server.on("/status", handleStatus);
  server.begin();

  Serial.println("=== WiFi SoftAP ===");
  Serial.printf("Rede: %s  Senha: %s\n", WIFI_SSID, WIFI_PASS);
  Serial.print("http://");
  Serial.println(WiFi.softAPIP());
}

void setup() {
  Serial.begin(115200);

  for (uint8_t m = 0; m < 2; m++) {
    digitalWrite(pinosRele[m][0], RELE_NC);
    digitalWrite(pinosRele[m][1], RELE_NC);
    pinMode(pinosRele[m][0], OUTPUT);
    pinMode(pinosRele[m][1], OUTPUT);
    parado(m);
  }

  digitalWrite(PIN_LED_MODO, RELE_NC);
  pinMode(PIN_LED_MODO, OUTPUT);
  pinMode(PIN_OLHO_BRANCO, OUTPUT);
  pinMode(PIN_OLHO_VERMELHO, OUTPUT);
  digitalWrite(PIN_OLHO_BRANCO, LOW);
  digitalWrite(PIN_OLHO_VERMELHO, LOW);
  setModo(false);  // normal: branco on, vermelho off, rele NC

  pinMode(PIN_PIR, INPUT_PULLDOWN);
  pirLastHighMs = 0;
  if (digitalRead(PIN_PIR) == HIGH) {
    pirLastHighMs = millis();
    presenca = true;
    setModo(true);
  }

  attachServos();
  setupWifiPortal();
  Serial.println("PIR GPIO18 | Tx no minimo (anti-horario) | aquecimento ~1 min");
}

void loop() {
  server.handleClient();
  atualizarPir();
}
