#include <avr/io.h>
#include <util/delay.h>
#include <NeoSWSerial.h>

#define RXC PD3   //RX_Command
#define TXC PD4   //TX_Command
#define RXB PD5   //RX_Bus
#define TXB PD6   //TX_Bus

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

NeoSWSerial busSerial(RXB, TXB);
NeoSWSerial ctrSerial(RXC, TXC);

byte ramMemory[16] = {  // 현재 LDA->LDB->ADD 실행구조 구현중. LDA는 전원이 켜지면 먼저 동작하는 기본 명령어다.
  0b00101100, //LDA에서 실행되는 부분이니까 상위 4비트가 LDB를 부르는 비트여야함.
  0b00111101,
  0b00100000,
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00000000,
  0b00000000, 
  0b00000000, 
  0b00000000, 
  0b00000000,
  0b00000101, //LDA에서 A에 저장될 값.
  0b00000011, //LDB에서 B에 저장될 값.
  0b00000000, 
  0b00000000
};      //즉 현재 목표는 LDB에서 ADD로 전환하는 것이다. 

#define CMD_RAM_TO_IR   0b00000100
#define CMD_RAM_TO_A    0b00000110
#define CMD_RAM_TO_B    0b00000111

byte dataToSend = 0b00000000;

void setup() {
  Serial.begin(19200);
  busSerial.begin(19200);
  ctrSerial.begin(19200);

  LED_OUTPUT;
  SB_SERIAL_HIGH_;
  SC_SERIAL_HIGH_Z;
  H_SERIAL_HIGH_Z;
  
  LED_LOW;
  ctrSerial.listen;
}

void loop() {
  if (Serial.available()) {
    byte address = Serial.read() & 0b00001111;
    
    if (address >= 16) address = 0;
    
    dataToSend = ramMemory[address];

    LED_HIGH;
    _delay_ms(200);
    LED_LOW;
  }
  
  if (ctrSerial.available()) {
    handleCommand(ctrSerial.read());
  }
}

void handleCommand(byte cmd) {
  switch(cmd) {
    case CMD_RAM_TO_IR:
    case CMD_RAM_TO_A:
    case CMD_RAM_TO_B:
      sendDataToBus();
      break;
      
    default:
      ctrSerial.listen();
      break;
  }
}

void sendDataToBus() {
  SB_SERIAL_OUTPUT;
  SB_SERIAL_TXB_LOW;
  _delay_ms(50);
  
  busSerial.listen();
  _delay_ms(10);
  
  busSerial.write(dataToSend);
  busSerial.flush();
  
  _delay_ms(15);
  
  SB_SERIAL_HIGH_Z;
  
  ctrSerial.listen;
}
