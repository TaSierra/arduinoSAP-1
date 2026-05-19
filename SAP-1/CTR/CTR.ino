#include <NeoSWSerial.h>
#include <avr/io.h>
#include <util/delay.h>

#define CLKpin PD2

#define TXC PD4     //TX_Command
#define RXC PD3     //RX_Command

#define S_SERIAL_HIGH_Z \
    DDRD  &= ~((1 << TXC) | (1 << RXC)); \
    PORTD &= ~((1 << TXC) | (1 << RXC));

#define S_SERIAL_OUTPUT \
    DDRD  |= ((1 << TXC) | (1 << RXC)); \
    PORTD &= ~((1 << TXC) | (1 << RXC));

#define H_SERIAL_HIGH_Z \
    DDRD  &= ~((1 << PD1) | (1 << PD0)); \
    PORTD &= ~((1 << PD1) | (1 << PD0));

#define H_SERIAL_OUTPUT \
    DDRD  |= ((1 << PD1) | (1 << PD0)); \
    PORTD &= ~((1 << PD1) | (1 << PD0));



#define PC_CLEAR        0b00000000
#define PC_INC          0b00000001
#define PC_to_MAR       0b00000010
#define MAR_to_RAM      0b00000011
#define RAM_to_IR       0b00000100
#define IR_to_MAR       0b00000101
#define RAM_to_A        0b00000110
#define RAM_to_B        0b00000111
#define ALU_ADD         0b00001001
#define A_to_OUT        0b00001011
#define ALU_to_A        0b00001100
#define A_to_ALU        0b00011000
#define B_to_ALU        0b00011100

#define CYCLE_LDA  0b0001
#define CYCLE_LDB  0b0010
#define CYCLE_ADD  0b0011

#define LED_OUTPUT DDRB |= (1 << 5);
#define LED_HIGH  PORTB |= (1 << 5);
#define LED_LOW   PORTB &= ~(1 << 5);

byte stp = 0;
bool lastCLK = LOW;
byte currentCycle = CYCLE_LDA;
byte nextInputOP = CYCLE_LDA;

NeoSWSerial ctrSerial(RXC, TXC);

void setup() {
  Serial.begin(19200);
  ctrSerial.begin(19200);

  DDRD &= ~(1 << CLKpin);
  S_SERIAL_HIGH_Z;
  LED_OUTPUT;

  _delay_ms(100);
  lastCLK = PIND & (1 << CLKpin);
}

void loop() {
  if (Serial.available()) {
    byte opcode = Serial.read();
    nextInputOP = (opcode >> 4) & 0b00001111;

    if(opcode == 0b0011){
      LED_HIGH;
      _delay_ms(100);
      LED_LOW;
    }
  }

  bool currentCLK = PIND & (1 << CLKpin);
  if (lastCLK == LOW && currentCLK == HIGH) {
    stp++;
    bool isCycleComplete = false;

    switch(currentCycle) {
      case CYCLE_LDA:
        isCycleComplete = LDAcycle(stp);
        break;
      case CYCLE_LDB:
        isCycleComplete = LDBcycle(stp);
        break;
      case CYCLE_ADD:
        isCycleComplete = ADDcycle(stp);
        break;
    }
    if (isCycleComplete) {
      stp = 0;
      currentCycle = nextInputOP;
    }
  }
  lastCLK = currentCLK;
}

bool LDAcycle(byte step) {
  switch (step) {
    case 1: sendCommand(PC_to_MAR); break;
    case 2: sendCommand(PC_INC); break;
    case 3: sendCommand(RAM_to_IR); break;        //case5와 case6 사이에 옵코드 전송이 있음.
    case 4: sendCommand(IR_to_MAR); break;
    case 5: sendCommand(MAR_to_RAM); break;
    case 6: sendCommand(RAM_to_A); break;         // 마지막 명령 실행
    case 7: return true;                           // idle 클럭 후 사이클 완료
    default: break;
  }
  return false;
}

bool LDBcycle(byte step) {
  switch (step) {
    case 1: sendCommand(PC_to_MAR); break;
    case 2: sendCommand(PC_INC); break;
    case 3: sendCommand(RAM_to_IR); break;      //case4와 case5 사이
    case 4: sendCommand(IR_to_MAR); break;
    case 5: sendCommand(MAR_to_RAM); break;
    case 6: sendCommand(RAM_to_B); break;        // 마지막 명령 실행
    case 7: return true;                         // idle 클럭 후 사이클 완료
    default: break;
  }
  return false;
}

bool ADDcycle(byte step) {
  switch (step) {
    case 1: sendCommand(A_to_ALU); break;
    case 2: sendCommand(B_to_ALU); break;
    case 3: sendCommand(ALU_ADD); break;
    case 4: sendCommand(ALU_to_A); break;
    case 5: sendCommand(PC_to_MAR); break;
    case 6: sendCommand(PC_INC); break;
    case 7: sendCommand(RAM_to_IR); break;
    case 8: sendCommand(A_to_OUT); break;       // 마지막 명령 실행
    case 9: return true;                         // idle 클럭 후 사이클 완료
    default: break;
  }
  return false;
}

void sendCommand(byte cmd) {
  S_SERIAL_OUTPUT;

  //_delay_ms(5);

  //ctrSerial.listen();
  _delay_ms(10);

  ctrSerial.write(cmd);
  ctrSerial.flush();

  _delay_ms(15);

  S_SERIAL_HIGH_Z;
  _delay_ms(5);
}
