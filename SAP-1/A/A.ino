#include <avr/io.h>
#include <util/delay.h>
#include <NeoSWSerial.h>

#define RXC PD3   //RX_Command
#define TXC PD4   //TX_Command
#define RXB PD5   //RX_Bus
#define TXB PD6   //TX_Bus

NeoSWSerial busSerial(RXB, TXB);
NeoSWSerial ctrSerial(RXC, TXC);

#define SB_SERIAL_HIGH_Z \
    DDRB  &= ~((1 << TXB) | (1 << RXB)); \
    PORTB &= ~((1 << TXB) | (1 << RXB));

#define SB_SERIAL_OUTPUT \
    DDRD  |= ((1 << TXB) | (1 << RXB)); \
    PORTD &= ~((1 << TXB) | (1 << RXB));

#define SB_SERIAL_TXB_LOW PORTD &= ~(1 << TXB);

#define SC_SERIAL_HIGH_Z \
    DDRD  &= ~((1 << TXC) | (1 << RXC)); \
    PORTD &= ~((1 << TXC) | (1 << RXC));

#define SC_SERIAL_OUTPUT \
    DDRD  |= ((1 << TXC) | (1 << RXC)); \
    PORTD &= ~((1 << TXC) | (1 << RXC));

#define H_SERIAL_HIGH_Z \
    DDRD  &= ~((1 << PD1) | (1 << PD0)); \
    PORTD &= ~((1 << PD1) | (1 << PD0));

#define H_SERIAL_OUTPUT \
    DDRD  |= ((1 << PD1) | (1 << PD0)); \
    PORTD &= ~((1 << PD1) | (1 << PD0));



#define LED_OUTPUT DDRB |= (1 << 5);
#define LED_HIGH  PORTB |= (1 << 5);
#define LED_LOW   PORTB &= ~(1 << 5);


#define CMD_RAM_TO_A   0b00000110
#define CMD_A_TO_ALU   0b00011000
#define CMD_ALU_TO_A   0b00001100
#define CMD_A_TO_OUT   0b00001011

byte aValue = 0b00000000;

void setup() {
  Serial.begin(19200);
  busSerial.begin(19200);
  ctrSerial.begin(19200);

  LED_OUTPUT;
  SB_SERIAL_HIGH_Z;
  SC_SERIAL_HIGH_Z;
  H_SERIAL_HIGH_Z;

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
    case CMD_RAM_TO_A:
      receiveFromBus();
      break;

    case CMD_ALU_TO_A:
      receiveFromALU();
      break;

    case CMD_A_TO_ALU:
      sendToALU();
      break;

    case CMD_A_TO_OUT:
      sendToBus();
      break;
  }

  ctrSerial.listen();
}

void receiveFromBus() {
  while(busSerial.available()) {
    busSerial.read();
  }

  busSerial.listen();

  for(int i = 0; i < 300; i++) {
    if (busSerial.available()) {
      aValue = busSerial.read();

      if (aValue == 5) {
        LED_HIGH;
        _delay_ms(100);
        LED_LOW;
      }
      else if (aValue == 3) {
        for(int j=0; j<2; j++) {
          LED_HIGH;
          _delay_ms(100);
          LED_LOW;
          _delay_ms(100);
        }
      } else if (aValue == 8) {
        LED_HIGH;
        _delay_ms(1000);
        LED_LOW;
      } else {
        byte blinkCount = aValue;

        if (blinkCount > 5) blinkCount = 5;

        for(int j=0; j<blinkCount; j++) {
          LED_HIGH;
          _delay_ms(100);
          LED_LOW;
          _delay_ms(100);
        }
      }

      ctrSerial.listen();
      return;
    }
    _delay_ms(10);
  }

  ctrSerial.listen();
}

void receiveFromALU() {
  while(Serial.available()) {
    Serial.read();
  }

  for(int i = 0; i < 300; i++) {
    if (Serial.available()) {
      aValue = Serial.read();

      LED_HIGH;
      _delay_ms(200);
      LED_LOW;

      return;
    }
    _delay_ms(10);
  }
}

void sendToALU() {
  H_SERIAL_OUTPUT;
  Serial.write(aValue);
  Serial.flush();

  _delay_ms(50);
  H_SERIAL_HIGH_Z;
}

void sendToBus() {
  SB_SERIAL_OUTPUT;
  SB_SERIAL_TXB_LOW;
  _delay_ms(5);

  busSerial.listen();
  _delay_ms(10);

  busSerial.write(aValue);
  busSerial.flush();

  _delay_ms(15);

  SB_SERIAL_TXB_LOW;
  _delay_ms(5);
  SB_SERIAL_HIGH_Z;
}
