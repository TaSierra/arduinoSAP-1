#include <avr/io.h>
#include <util/delay.h>
#include <NeoSWSerial.h>

#define RXC PD3   //RX_Command
#define TXC PD4   //TX_Command
#define RXB PD5   //RX_Bus
#define TXB PD6   //TX_Bus

NeoSWSerial ctrSerial(RXC, TXC);
NeoSWSerial bSerial(RXB, TXB);

#define CMD_A_TO_ALU   0b00011000
#define CMD_B_TO_ALU   0b00011100
#define CMD_ALU_ADD    0b00001001
#define CMD_ALU_TO_A   0b00001100

#define SB_SERIAL_HIGH_Z \
    DDRD  &= ~((1 << TXB) | (1 << RXB)); \
    PORTD &= ~((1 << TXB) | (1 << RXB));

#define SB_SERIAL_OUTPUT \
    DDRD  |= ((1 << TXB) | (1 << RXB)); \
    PORTD &= ~((1 << TXB) | (1 << RXB));

#define SB_SERIAL_TXB_LOW PORTD &= ~(1 << TXB);


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

#define CMD_RAM_TO_A   0b00000110
#define CMD_A_TO_ALU   0b00011000
#define CMD_ALU_TO_A   0b00001100
#define CMD_A_TO_OUT   0b00001011

byte aluA = 0b00000000;
byte aluB = 0b00000000;
byte aluResult = 0b00000000;

void setup() {
  Serial.begin(19200);
  ctrSerial.begin(19200);
  bSerial.begin(19200);

  LED_OUTPUT;
  SC_SERIAL_HIGH_Z;
  SB_SERIAL_HIGH_Z;
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
    case CMD_A_TO_ALU:
      receiveFromA();
      break;

    case CMD_B_TO_ALU:
      receiveFromB();
      break;

    case CMD_ALU_ADD:
      executeAdd();
      break;

    case CMD_ALU_TO_A:
      sendToA();
      break;
  }

  ctrSerial.listen();
}

void receiveFromA() {
  while(Serial.available()) {
    Serial.read();
  }

  unsigned long waitStart = millis();
  while (!Serial.available() && (millis() - waitStart < 500)) {}

  if (Serial.available()) {
    aluA = Serial.read();

    LED_HIGH;
    _delay_ms(100);
    LED_LOW;
  }
}

void receiveFromB() {
  while(bSerial.available()) {
    bSerial.read();
  }

  bSerial.listen();

  unsigned long waitStart = millis();
  while (!bSerial.available() && (millis() - waitStart < 500)) {}

  if (bSerial.available()) {
    aluB = bSerial.read();

    LED_HIGH;
    _delay_ms(100);
    LED_LOW;
  }

  ctrSerial.listen();
}

void executeAdd() {
  aluResult = aluA + aluB;

  LED_HIGH;
  _delay_ms(200);
  LED_LOW;
}

void sendToA() {
  H_SERIAL_OUTPUT;
  Serial.write(aluResult);
  Serial.flush();

  _delay_ms(50);
  H_SERIAL_HIGH_Z;
}
