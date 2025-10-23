#include <Joystick.h>

#define PPM_PIN 2          // pino de entrada do sinal PPM
#define CHANNELS 8         // número de canais
#define SYNC_GAP 4000      // gap mínimo (µs) para identificar novo frame

volatile uint16_t ppmValues[CHANNELS];
volatile byte currentChannel = 0;
volatile unsigned long lastTime = 0;

// Objeto joystick USB (até 8 eixos)
Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,
                   JOYSTICK_TYPE_JOYSTICK,
                   0, 0, // sem botões
                   true, true, true, true, true, true, true, true);

void setup() {
  pinMode(PPM_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(PPM_PIN), ppmISR, RISING);

  Joystick.begin();
}

void loop() {
  // Copia valores de forma segura (sem interferir na interrupção)
  noInterrupts();
  uint16_t channels[CHANNELS];
  for (byte i = 0; i < CHANNELS; i++) channels[i] = ppmValues[i];
  interrupts();

  // Atualiza eixos do joystick
  for (int i = 0; i < CHANNELS; i++) {
    int value = constrain(channels[i], 1000, 2000);     // garante faixa segura
    int mapped = map(value, 1000, 2000, 0, 1023);       // 10 bits (0–1023)
    switch (i) {
      case 0: Joystick.setXAxis(mapped); break;
      case 1: Joystick.setYAxis(mapped); break;
      case 2: Joystick.setZAxis(mapped); break;
      case 3: Joystick.setRxAxis(mapped); break;
      case 4: Joystick.setRyAxis(mapped); break;
      case 5: Joystick.setRzAxis(mapped); break;
      case 6: Joystick.setThrottle(mapped); break;
      case 7: Joystick.setRudder(mapped); break;
    }
  }

  delay(20); // ~50 Hz de atualização
}

// --- Interrupção para decodificar PPM ---
void ppmISR() {
  unsigned long now = micros();
  unsigned int diff = now - lastTime;
  lastTime = now;

  if (diff > SYNC_GAP) {
    currentChannel = 0;  // início de novo frame
  } else if (currentChannel < CHANNELS) {
    ppmValues[currentChannel++] = diff;
  }
}
