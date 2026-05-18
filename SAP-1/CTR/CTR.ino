/*
 * Controller&Sequensor
 */
#include <NeoSWSerial.h>

int CLKpin = 2;  
int TXC = 4;
int RXC = 3;

#define PC_CLEAR        0b00000000
#define PC_INC          0b00000001
#define PC_to_MAR       0b00000010
#define byte MAR_to_RAM 0b00000011
#define RAM_to_IR       0b00000100
#define IR_to_MAR       0b00000101
#define RAM_to_A        0b00000110
#define RAM_to_B        0b00000111
#define ALU_ADD         0b00001001
#define A_to_OUT        0b00001011
#define ALU_to_A        0b00001100
#define A_to_ALU        0b00011000
#define byte B_to_ALU   0b00011100

#define CYCLE_LDA  0b0001
#define CYCLE_LDB  0b0010
#define CYCLE_ADD  0b0011

byte stp = 0;
bool lastCLK = LOW;
byte currentCycle = CYCLE_LDA;
byte nextInputOP = CYCLE_LDA;

NeoSWSerial ctrSerial(RXC, TXC);

void setup() {
  Serial.begin(19200);
  ctrSerial.begin(19200);
  pinMode(CLKpin, INPUT);
  pinMode(TXC, INPUT);
  pinMode(13, OUTPUT);
  pinMode(0, INPUT);
  digitalWrite(13, LOW);
  
  delay(100);
  lastCLK = digitalRead(CLKpin);
}

void loop() {
  if (Serial.available()) {
    byte opcode = Serial.read();
    nextInputOP = (opcode >> 4) & 0b00001111;
    
    if(opcode == 0b0011){
      digitalWrite(13, HIGH);
      delay(100);
      digitalWrite(13, LOW);
    }
  }
  
  bool currentCLK = digitalRead(CLKpin);
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
  pinMode(TXC, OUTPUT);
  digitalWrite(TXC, LOW);
  delay(5);
  
  ctrSerial.listen();
  delay(10);
  
  ctrSerial.write(cmd);
  ctrSerial.flush();
  
  delay(15);
  
  digitalWrite(TXC, LOW);
  delay(5);
  pinMode(TXC, INPUT);
}
