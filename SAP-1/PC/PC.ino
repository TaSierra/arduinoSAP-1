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


#define LED_OUTPUT DDRB |= (1 << 5);
#define LED_HIGH  PORTB |= (1 << 5);
#define LED_LOW   PORTB &= ~(1 << 5);

NeoSWSerial busSerial(RXB, TXB);
NeoSWSerial ctrSerial(RXC, TXC);

#define CMD_PC_INC     0b00000001
#define CMD_PC_TO_MAR  0b00000010

byte pcValue = 0b0000;

void setup() {
  busSerial.begin(19200);
  ctrSerial.begin(19200);

  SB_SERIAL_HIGH_Z;
  LED_OUTPUT;

  ctrSerial.listen();
  LED_LOW;
}

void loop() {
  if (ctrSerial.available()) {
    byte cmd = ctrSerial.read();

    if (cmd == CMD_PC_INC) {
      pcValue++;
      LED_HIGH;
      _delay_ms(200);
      LED_LOW;
    }
    else if (cmd == CMD_PC_TO_MAR) {
      SB_SERIAL_OUTPUT;
      _delay_ms(10);

      busSerial.write(pcValue);
      busSerial.flush();
      _delay_ms(10);

      SB_SERIAL_HIGH_Z;
      ctrSerial.listen();

      if(pcValue == 0b0001) {
        LED_HIGH;
        _delay_ms(100);
        LED_LOW;
      }
    }
  }
}
