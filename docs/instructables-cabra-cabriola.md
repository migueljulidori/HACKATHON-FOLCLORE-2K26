# Cabra Cabriola Mecânica: Folclore Brasileiro + ESP32 (Hackathon 2k26)

**Publicado no Instructables:**  
https://www.instructables.com/Prototipagem-Da-Lenda-Folclórica-Cabra-Cabriola-Em/

**By:** equipe Cabra Cabriola — Hackathon 2k26  
**Tema do hackathon:** *O Folclore Brasileiro: nova skin, novos poderes*  
**Tempo estimado:** 1–2 dias (estrutura + eletrônica + firmware)  
**Dificuldade:** intermediário  
**Custo aproximado:** baixo/médio (papelão + ESP32 + motores TT + servos + sensores)

---

## Introdução

A **Cabra Cabriola** é uma figura assustadora das histórias populares brasileiras. Neste projeto, reinterpretamos a lenda como um **boneco mecânico** de papelão: chifres curvos, olhos que mudam de cor, braços articulados, boca móvel e deslocamento sobre rodas — controlado por celular via Wi-Fi.

A ideia une **tradição oral** e **cultura maker**: a “nova skin” é visual (papelão + LEDs vermelhos) e os “novos poderes” são tecnológicos (detecção de presença, modo bravo automático e teleoperação por página web).

Neste Instructable documentamos **como prototipar** a criatura do zero: estrutura, eletrônica, pinagem, firmware e uso.

**[FOTO]** Hero: Cabra Cabriola montada, olhos vermelhos acesos, ambiente de hackathon.

---

## O que ela faz

- **Andar / girar** — 2 motores redutores amarelos (TT) + módulos de relé
- **Mexer braços e boca** — 3 servos MG996R (180°)
- **Modo normal / bravo** — LEDs dos olhos (branco ↔ vermelho) + relé de painel
- **Presença** — sensor PIR HC-SR501 ativa o modo bravo sozinho
- **Controle** — ESP32 cria Wi-Fi; celular abre http://192.168.4.1 com joystick

**[FOTO]** Tela do celular com a página de controle (joystick + modos).

---

## Materiais (Supplies)

### Estrutura / estética

- Papelão grosso (corpo, cabeça, chifres curvos)
- Cola quente, fita, estilete, régua
- Tinta / caneta (detalhes assombrosos)
- Base com espaço para motores e eletrônica

### Eletrônica

- 1× ESP32 (DevKit 38 pinos)
- 2× motores redutores amarelos TT (ou similar 3–6 V)
- 2× módulos de 2 relés 5 V (ponte H por motor) + 1 canal/módulo para LED de painel
- 3× servos MG996R (braço E, braço D, boca)
- 4× LEDs para olhos: 2 brancos + 2 vermelhos
- 4× resistores 220 Ω (olhos)
- 1× sensor de presença HC-SR501
- Fonte 5–6 V (≥ 3–5 A recomendado para servos)
- Fonte dos motores TT (conforme o motor, tipicamente 3–6 V)
- Fonte 12 V se o LED de painel/fita for 12 V (só nos contatos do relé)
- Protoboard / jumpers, GND comum entre fontes e ESP32

### Software

- Arduino IDE
- Core ESP32
- Biblioteca ESP32Servo
- Sketch: sketch_oct30a (SoftAP + web + joystick)

**[FOTO]** Mesa com componentes separados (ESP32, relés, motores, servos, PIR, LEDs).

---

## Passo 1 — Conceito e “nova skin”

1. Esboce a Cabra Cabriola: cabeça com chifres curvos, tronco, braços e base móvel.
2. Defina a linguagem visual: papelão aparente + olhos vermelhos no modo bravo (tom de boneco mecânico / assombro).
3. Planeje volumes ocos para caber ESP32, fontes e cabos sem travar as articulações.

A narrativa do protótipo: em repouso ela parece “neutra” (olhos brancos); quando sente presença, entra em modo bravo (olhos vermelhos) — um “novo poder” ligado ao folclore do susto.

**[FOTO]** Croqui em papel + primeira peça de papelão dos chifres.

---

## Passo 2 — Estrutura de papelão

1. Corte o corpo e a cabeça em camadas ou caixas reforçadas.
2. Fixe os chifres com encaixe + cola quente (curvatura bem marcada).
3. Deixe furos para:
   - eixos/rodas dos motores TT
   - servos dos braços e da boca
   - LEDs dos olhos
   - lente do PIR (sem cobrir demais)
4. Monte uma base estável: motores nas laterais, eletrônica no centro, cabos organizados.

**[FOTO]** Estrutura aberta mostrando compartimento interno.  
**[FOTO]** Detalhe dos chifres e encaixe da cabeça.

Dica: deixe painéis removíveis (tampa traseira) para depurar fiação sem desmontar a criatura.

---

## Passo 3 — Locomoção (2 motores TT + relés)

Cada motor usa 1 módulo de 2 relés em ponte H (polaridade invertida = frente/trás).

### Motor esquerdo — módulo GPIO32 / 33

- IN1 → GPIO32
- IN2 → GPIO33
- Fio 1 do motor → COM do IN1
- Fio 2 do motor → COM do IN2
- NC do IN1 → +Vmotor
- NO do IN1 → GND do motor
- NC do IN2 → GND do motor
- NO do IN2 → +Vmotor (cruzado)

### Motor direito — módulo GPIO25 / 26

- IN1 → GPIO25
- IN2 → GPIO26
- Mesma lógica de ponte H cruzada do motor esquerdo

Também:

- JD-VCC dos módulos → 5 V (jumper conforme sua placa)
- GND comum com o ESP32
- Fonte do motor não deve alimentar o ESP pelo mesmo 5 V frágil da USB se a corrente for alta

No firmware, o joystick da página faz:

- frente / trás → os dois no mesmo sentido
- direita → esquerdo frente + direito trás
- esquerda → o inverso

**[FOTO]** Módulos de relé sob a base + motores TT nas laterais.

---

## Passo 4 — Articulações (servos MG996R)

- Boca → sinal GPIO13 · alimentação 5–6 V externo + GND comum
- Braço esquerdo → sinal GPIO27 · mesma alimentação
- Braço direito → sinal GPIO14 · mesma alimentação

Importante: nunca alimente MG996R pelo 5 V do ESP32. Use fonte externa e una os GNDs.

No código, ângulos 0–180°; a boca usa posições de aberta/fechada; braços têm centro em 90°. Se um servo girar ao contrário na estrutura, inverta no firmware (180 − ângulo) sem remonter.

**[FOTO]** Servo do braço embutido no papelão.  
**[FOTO]** Mecanismo simples da boca (haste/cola quente).

---

## Passo 5 — Olhos (branco × vermelho) e modo bravo

Cada olho leva 1 LED branco + 1 LED vermelho.

Ligações:

- LED branco esquerdo → resistor 220 Ω → GPIO19 · catodo → GND
- LED branco direito → resistor 220 Ω → GPIO19 · catodo → GND
- LED vermelho esquerdo → resistor 220 Ω → GPIO21 · catodo → GND
- LED vermelho direito → resistor 220 Ω → GPIO21 · catodo → GND

Comportamento:

- Modo normal → brancos acesos, vermelhos apagados
- Modo bravo → vermelhos acesos, brancos apagados

Opcional: um relé no GPIO23 comuta um LED/fita de painel (NC = calmo, NO = bravo), se quiser luz de cena além dos olhos.

**[FOTO]** Olhos no escuro: modo normal (branco) vs bravo (vermelho).

---

## Passo 6 — Sensor de presença HC-SR501

- VCC do sensor → 5 V do ESP (ou fonte 5 V)
- GND do sensor → GND
- OUT do sensor → GPIO18

### Potenciômetros e jumper

1. Jumper em H (retrigger) — presença contínua reinicia o tempo.
2. Tx (tempo): gire anti-horário no mínimo (~3–5 s) para o modo bravo não “grudar”.
3. Sx (sensibilidade): comece no meio; se dispara sozinho, diminua; se não detecta, aumente.
4. Após ligar, espere ~30–60 s de aquecimento.

Comportamento: presença → modo bravo automático; sem presença → volta ao normal.

**[FOTO]** PIR embutido no peito/cabeça com lente visível.  
**[FOTO]** Parte de trás do módulo mostrando pots Tx/Sx e jumper H.

---

## Passo 7 — Firmware e Wi-Fi SoftAP

1. Instale o core ESP32 e a biblioteca ESP32Servo.
2. Abra o sketch do projeto (sketch_oct30a).
3. Placa: ESP32 Dev Module.
4. Grave e abra o Monitor Serial (115200).

O ESP32 sobe o ponto de acesso:

- Rede: Robo-ESP32
- Senha: 12345678
- Página: http://192.168.4.1

Na página:

- Joystick de locomoção
- Normal / Bravo (manual)
- Boca abrir/fechar
- Sliders dos braços
- Indicador de presença do PIR

**[FOTO]** Serial mostrando rede SoftAP + IP.  
**[FOTO]** Celular conectado na rede do robô.

---

## Passo 8 — Integração final e testes

Checklist antes da demo do hackathon:

1. GND comum (ESP + fontes + módulos).
2. Servos em fonte forte; motores em fonte adequada.
3. Teste joystick: frente, trás, giro E/D.
4. Teste braços/boca sem colidir com o papelão.
5. Teste olhos: Normal ↔ Bravo.
6. Teste PIR após 1 minuto: aproximação → olhos vermelhos.
7. Organize cabos para a estrutura fechar sem pinçar fios.

**[FOTO]** Demo completa: pessoa se aproxima, olhos vermelhos, braços em pose.

---

## Resumo de pinagem (ESP32)

- GPIO 32 e 33 — relés motor esquerdo
- GPIO 25 e 26 — relés motor direito
- GPIO 23 — relé modo painel (opcional)
- GPIO 19 — LEDs brancos (olhos)
- GPIO 21 — LEDs vermelhos (olhos)
- GPIO 18 — PIR OUT
- GPIO 13 — servo boca
- GPIO 27 — servo braço esquerdo
- GPIO 14 — servo braço direito

---

## Problemas comuns

- Relé de motor clica mas não anda — confira +Vmotor, GND e cruzamento NC/NO.
- Servo treme / reseta o ESP — use fonte externa ≥ 3–5 A e GND comum.
- PIR lento para apagar — pot Tx alto; deixe Tx no mínimo (anti-horário).
- Olhos fracos — use 220 Ω no ânodo e catodo no GND.
- Página não abre — conecte só no Wi-Fi Robo-ESP32 e abra 192.168.4.1 (desligue dados móveis se atrapalhar).
- Braço invertido — troque o GPIO no código ou use 180 − ângulo.

---

## Conclusão — folclore + maker

A Cabra Cabriola deste protótipo não é só um robô: é uma releitura da lenda. O papelão traz a estética popular e improvisada; o ESP32, os olhos vermelhos e o sensor de presença traduzem o “susto” da tradição em interação contemporânea.

No espírito do Hackathon 2k26 — nova skin, novos poderes — a skin é visual e material; o poder é a presença que desperta o modo bravo, operável ainda por qualquer visitante via Wi-Fi.

### Próximos poderes (ideias)

- Áudio com DFPlayer (voz / berro da cabra)
- Sequências automáticas de braços no modo bravo
- Luzes e fumaça teatral (com segurança)

**[FOTO]** Equipe com a Cabra Cabriola no estande do hackathon.

---

## Créditos

- Evento: Hackathon 2k26 — Folclore Brasileiro
- Personagem: Cabra Cabriola (folclore popular brasileiro)
- Protótipo: estrutura em papelão + ESP32 + motores TT + MG996R + PIR + controle SoftAP
- Firmware de referência: sketch_oct30a

---

## Licença sugerida no Instructables

Share Alike / Attribution — use, remixe e cite a equipe e o hackathon.

Substitua os marcadores [FOTO] por imagens reais ao publicar no Instructables.
