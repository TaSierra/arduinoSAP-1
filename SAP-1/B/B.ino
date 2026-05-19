#include <avr/io.h>
#include <util/delay.h>
#include <NeoSWSerial.h>

#define RXB  PD5
#define TXB  PD6
#define RXC  PD3
#define TXC  PD4
#define RXA  PD7 //ALU RX
#define TXA  PB0 //ALU TX

#define SB_SERIAL_HIGH_Z \
    DDRD  &= ~((1 << TXB) | (1 << RXB)); \
    PORTD &= ~((1 << TXB) | (1 << RXB));

#define SB_SERIAL_OUTPUT \
    DDRD  |= ((1 << TXB) | (1 << RXB)); \
    PORTD &= ~((1 << TXB) | (1 << RXB));

#define SB_SERIAL_TXB_LOW PORTD &= ~(1 << TXB);


#define SC_SERIAL_HIGH_Z \
    DDRD  &= ~((1 << TXC) | (1 << RXC)); \
    PORTD &= ~((1 << RXC) | (1 << TXC));

#define SA_SERIAL_OUTPUT \
    DDRD |= (1 << RXA); \
    DDRB |= (1 << TXA);
    PORTD &= ~(1 << RXA);
    PORTB &= ~(1 << TXA);

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
NeoSWSerial bSerial(RXA, TXA);

#define CMD_RAM_TO_B   0b00000111
#define CMD_B_TO_ALU   0b00011100

byte bValue = 0b00000000;

void setup() {
  busSerial.begin(19200);
  ctrSerial.begin(19200);
  bSerial.begin(19200);
  
  LED_OUTPUT();
  SB_SERIAL_HIGH_Z();
  SC_SERIAL_HIGH_Z();
  pinMode(RXA, INPUT);
  pinMode(TXA, INPUT);
  
  digitalWrite(13, LOW);
  ctrSerial.listen();
}

void loop() {
  if (ctrSerial.available()) {
    handleCommand(ctrSerial.read());
  }
}

void handleCommand(byte cmd) {
  switch(cmd) {
    case CMD_RAM_TO_B:
      receiveFromBus();
      break;
      
    case CMD_B_TO_ALU:
      sendToALU();
      break;
  }
}

void receiveFromBus() {
  while(busSerial.available()) {
    busSerial.read();
  }
  
  busSerial.listen();
  delay(500);
  
  for(int i = 0; i < 300; i++) {
    if (busSerial.available()) {
      bValue = busSerial.read();
      
      if(bValue = 0b00000011) {
        digitalWrite(13, HIGH);
        delay(200);
        digitalWrite(13, LOW);
      }
      
      ctrSerial.listen();
      return;
    }
    delay(10);
  }
  
  ctrSerial.listen();
}

void sendToALU() {
  pinMode(TXA, OUTPUT);
  digitalWrite(TXA, LOW);
  delay(50);
  
  bSerial.listen();
  delay(10);
  
  bSerial.write(bValue);
  bSerial.flush();
  digitalWrite(13, HIGH);
  
  delay(15);
  
  digitalWrite(TXA, LOW);
  delay(5);
  pinMode(TXA, INPUT);
  
  ctrSerial.listen();
  delay(200);
  digitalWrite(13, LOW);
}
