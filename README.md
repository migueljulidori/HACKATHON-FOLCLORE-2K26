# HACKATHON-FOLCLORE-2K26 — Cabra Cabriola

Protótipo maker da **Cabra Cabriola** para o **Hackathon 2k26**.

**Tema:** *O Folclore Brasileiro: nova skin, novos poderes*

Reinterpretamos a lenda da Cabra Cabriola como um **boneco mecânico de papelão**: chifres curvos, olhos que mudam de cor, braços e boca articulados, locomoção por motores e controle por Wi‑Fi no celular. A “nova skin” é visual; o “novo poder” é a interação (presença → modo bravo).

---

## O que tem neste repositório

```
HACKATHON-FOLCLORE-2K26/
├── README.md
├── docs/
│   └── instructables-cabra-cabriola.md   # tutorial (estilo Instructables)
└── firmware/
    ├── cabra-cabriola-controle/          # firmware principal (ESP32 SoftAP + web)
    ├── cabra-cabriola-articulacoes/      # experimento servos + passos (L298N)
    └── cabra-cabriola-voz/               # experimento DFPlayer Mini + áudios
```

| Pasta | Uso |
|-------|-----|
| `firmware/cabra-cabriola-controle` | **Firmware oficial do protótipo** (joystick Wi‑Fi, motores TT, servos, olhos, PIR, modo bravo) |
| `firmware/cabra-cabriola-articulacoes` | Versão anterior / testes com L298N + vários servos |
| `firmware/cabra-cabriola-voz` | Testes de áudio com DFPlayer Mini |
| `docs/instructables-cabra-cabriola.md` | Tutorial completo de prototipagem |

---

## Hardware (firmware principal)

- ESP32 DevKit (38 pinos)
- 2× motores redutores amarelos (TT) + 2× módulos de 2 relés (ponte H)
- 3× servos MG996R (boca + 2 braços)
- LEDs dos olhos: 2 brancos (GPIO19) + 2 vermelhos (GPIO21), resistores 220 Ω
- Relé de painel modo normal/bravo (GPIO23) — opcional
- Sensor PIR HC-SR501 (GPIO18)
- Fontes: 5–6 V (servos), 3–6 V (motores TT), GND comum com o ESP32

### Pinagem resumida

- GPIO 32 / 33 — relés motor esquerdo  
- GPIO 25 / 26 — relés motor direito  
- GPIO 23 — relé modo painel  
- GPIO 19 — LEDs brancos (olhos)  
- GPIO 21 — LEDs vermelhos (olhos)  
- GPIO 18 — PIR OUT  
- GPIO 13 — servo boca  
- GPIO 27 — servo braço esquerdo  
- GPIO 14 — servo braço direito  

Detalhes de ligação e montagem em papelão: ver `docs/instructables-cabra-cabriola.md`.

---

## Como gravar o firmware principal

1. Arduino IDE + core **ESP32** + biblioteca **ESP32Servo**
2. Abra `firmware/cabra-cabriola-controle/cabra-cabriola-controle.ino`
3. Placa: **ESP32 Dev Module**
4. Grave e abra o Serial (115200)

### Controle por celular

1. Conecte no Wi‑Fi **`Robo-ESP32`** (senha **`12345678`**)
2. Abra no navegador: **http://192.168.4.1**
3. Use o joystick, modos Normal/Bravo, boca e braços

**Modos**

- **Normal** — olhos brancos  
- **Bravo** — olhos vermelhos (manual na página ou automático pelo PIR)

---

## Tutorial Instructables

O arquivo `docs/instructables-cabra-cabriola.md` está pronto para colar no Instructables (sem tabelas Markdown, só listas). Substitua os marcadores `[FOTO]` pelas fotos reais do protótipo.

---

## Equipe / créditos

- **Evento:** Hackathon 2k26 — Folclore Brasileiro  
- **Personagem:** Cabra Cabriola (folclore popular brasileiro)  
- **Protótipo:** papelão + ESP32 + motores TT + MG996R + PIR + SoftAP  

---

## Licença

Conteúdo do projeto liberado para fins educacionais e do hackathon. Cite a equipe e o evento ao remixar.
