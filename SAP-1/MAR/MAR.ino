#include <avr/io.h>
#include <util/delay.h>
#include <NeoSWSerial.h>

#define RXC 3   //RX_Command
#define TXC 4   //TX_Command
#define RXB 5   //RX_Bus
#define TXB 6   //TX_Bus

#define SB_SERIAL_HIGH_Z \
    DDRB  &= ~((1 << TXB) | (1 << RXB)); \
    PORTB &= ~((1 << TXB) | (1 << RXB));

#define SB_SERIAL_OUTPUT \
    DDRB  |= ((1 << TXB) | (1 << RXB)); \
    PORTB &= ~((1 << TXB) | (1 << RXB));

#define SC_SERIAL_HIGH_Z \
    DDRD  &= ~((1 << TXC) | (1 << RXC)); \
    PORTD &= ~((1 << TXC) | (1 << RXC));

#define SC_SERIAL_OUTPUT \
    DDRC  |= ((1 << TXC) | (1 << RXC)); \
    PORTC &= ~((1 << TXC) | (1 << RXC));

#define H_SERIAL_HIGH_Z \
    DDRD  &= ~((1 << PD1) | (1 << PD0)); \
    PORTD &= ~((1 << PD1) | (1 << PD0));

#define H_SERIAL_OUTPUT \
    DDRD  |= ((1 << PD1) | (1 << PD0)); \
    PORTD &= ~((1 << PD1) | (1 << PD0));



#define LED_OUTPUT DDRB |= (1 << 5);
#define LED_HIGH  PORTB |= (1 << 5);
#define LED_LOW   PORTB &= ~(1 << 5);

NeoSWSerial busSerial(RXB, TXB);
NeoSWSerial ctrSerial(RXC, TXC);

#define CMD_PC_TO_MAR 0b00000010
#define CMD_IR_TO_MAR 0b00000101

byte marValue = 0b0000;

void setup() {
  Serial.begin(19200);

  busSerial.begin(19200);
  ctrSerial.begin(19200);

  LED_OUTPUT;

  SB_SERIAL_HIGH_Z();
  SC_SERIAL_HIGH_Z();

  LED_LOW;

  ctrSerial.listen();
}

void loop() {
  if (ctrSerial.available()) {
    handleCommand(ctrSerial.read());
  }
}

void handleCommand(byte cmd) {
  switch(cmd) {
    case CMD_PC_TO_MAR:
    case CMD_IR_TO_MAR:
      receiveFromBus();
      break;

    default:
      ctrSerial.listen();
      break;
  }
}

void receiveFromBus() {
  busSerial.listen();

  // 버퍼 비우기 (이전 쓰레기 값 제거)
  while(busSerial.available()) {
    busSerial.read();
  }

  // 데이터 수신 대기 (타임아웃 루프)
  for(int i = 0; i < 500; i++) {
    if (busSerial.available()) {
      marValue = busSerial.read() & 0b00001111; // 4비트 마스킹


      if(marValue == 0b0001) {
        LED_HIGH;

        sendToRAM();

        _delay_ms(500);
        LED_LOW;
      } else {
        // 0b0001이 아니면 그냥 전송만 하고 LED는 짧게 (혹은 안 킴)
        sendToRAM();
      }

      ctrSerial.listen();
      return;
    }
    _delay_ms(2);
  }

  // 타임아웃 시 복귀
  ctrSerial.listen();
}

void sendToRAM() {
  Serial.write(marValue);
  Serial.flush();
}
