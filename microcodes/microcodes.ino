#define SHIFT_CLK 25
#define SHIFT_LATCH 26
#define SHIFT_DATA 27

#define WRITE_ENABLE_AL 2 // 2 -> 34
#define OUTPUT_ENABLE_AL 4 // 4 -> 35

#define D0 15
#define D1 16
#define D2 17
#define D3 18
#define D4 19
#define D5 21
#define D6 12
#define D7 13

const int dataPins[] = { D0, D1, D2, D3, D4, D5, D6, D7 };

#define HLT 0b1000000000000000  // Halt clock
#define MI  0b0100000000000000  // Memory address register in
#define RI  0b0010000000000000  // RAM data in
#define RO  0b0001000000000000  // RAM data out
#define II  0b0000100000000000  // Instruction register in
#define AI  0b0000010000000000  // A register in
#define AO  0b0000001000000000  // A register out
#define EO  0b0000000100000000  // ALU out
#define SU  0b0000000010000000  // ALU subtract
#define BI  0b0000000001000000  // B register in
#define OI  0b0000000000100000  // Output register in
#define CE  0b0000000000010000  // Program counter enable
#define CO  0b0000000000001000  // Program counter out
#define J   0b0000000000000100  // Jump (program counter in)
#define FI  0b0000000000000010  // Flags in
#define TR  0b0000000000000001  // T-states reset

#define FLAGS_Z0C0 0
#define FLAGS_Z0C1 1
#define FLAGS_Z1C0 2
#define FLAGS_Z1C1 3

#define JC  0b0111
#define JZ  0b1000

const uint16_t UCODE_TEMPLATE[16][8] = {
  { MI|CO,  RO|II|CE,  TR,     0,         0,      0,            0,   0 },   // 0000 - NOP
  { MI|CO,  RO|II|CE,  MI|CO,  MI|RO|CE,  RO|AI,  TR,           0,   0 },   // 0001 - LDA
  { MI|CO,  RO|II|CE,  MI|CO,  MI|RO|CE,  RO|BI,  EO|AI|FI,     TR,  0 },   // 0010 - ADD
  { MI|CO,  RO|II|CE,  MI|CO,  MI|RO|CE,  RO|BI,  EO|AI|SU|FI,  TR,  0 },   // 0011 - SUB
  { MI|CO,  RO|II|CE,  MI|CO,  MI|RO|CE,  AO|RI,  TR,           0,   0 },   // 0100 - STA
  { MI|CO,  RO|II|CE,  MI|CO,  RO|AI|CE,  TR,     0,            0,   0 },   // 0101 - LDI
  { MI|CO,  RO|II|CE,  MI|CO,  RO|J,      TR,     0,            0,   0 },   // 0110 - JMP
  { MI|CO,  RO|II|CE,  CE,     TR,        0,      0,            0,   0 },   // 0111 - JC
  { MI|CO,  RO|II|CE,  CE,     TR,        0,      0,            0,   0 },   // 1000 - JZ
  { MI|CO,  RO|II|CE,  TR,     0,         0,      0,            0,   0 },   // 1001 - NOP
  { MI|CO,  RO|II|CE,  TR,     0,         0,      0,            0,   0 },   // 1010 - NOP
  { MI|CO,  RO|II|CE,  TR,     0,         0,      0,            0,   0 },   // 1011 - NOP
  { MI|CO,  RO|II|CE,  TR,     0,         0,      0,            0,   0 },   // 1100 - NOP
  { MI|CO,  RO|II|CE,  TR,     0,         0,      0,            0,   0 },   // 1101 - NOP
  { MI|CO,  RO|II|CE,  AO|OI,  TR,        0,      0,            0,   0 },   // 1110 - OUT
  { MI|CO,  RO|II|CE,  HLT,    0,         0,      0,            0,   0 },   // 1111 - HLT
};

uint16_t ucode[4][16][8];

void initUCode() {
  // ZF = 0, CF = 0
  memcpy(ucode[FLAGS_Z0C0], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));

  // ZF = 0, CF = 1
  memcpy(ucode[FLAGS_Z0C1], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[FLAGS_Z0C1][JC][2] = MI|CO;
  ucode[FLAGS_Z0C1][JC][3] = RO|J;
  ucode[FLAGS_Z0C1][JC][4] = TR;

  // ZF = 1, CF = 0
  memcpy(ucode[FLAGS_Z1C0], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[FLAGS_Z1C0][JZ][2] = MI|CO;
  ucode[FLAGS_Z1C0][JZ][3] = RO|J;
  ucode[FLAGS_Z1C0][JZ][4] = TR;

  // ZF = 1, CF = 1
  memcpy(ucode[FLAGS_Z1C1], UCODE_TEMPLATE, sizeof(UCODE_TEMPLATE));
  ucode[FLAGS_Z1C1][JC][2] = MI|CO;
  ucode[FLAGS_Z1C1][JC][3] = RO|J;
  ucode[FLAGS_Z1C1][JC][4] = TR;

  ucode[FLAGS_Z1C1][JZ][2] = MI|CO;
  ucode[FLAGS_Z1C1][JZ][3] = RO|J;
  ucode[FLAGS_Z1C1][JZ][4] = TR;
}

void setReadMode() {
  int pinCount = sizeof(dataPins) / sizeof(int);

  for (int pin = 0; pin < pinCount; pin += 1) {
    pinMode(dataPins[pin], INPUT);
  }
}

void setWriteMode() {
  int pinCount = sizeof(dataPins) / sizeof(int);

  for (int pin = 0; pin < pinCount; pin += 1) {
    pinMode(dataPins[pin], OUTPUT);
  }
}

void setAddress(int address) {
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, (address >> 8));
  shiftOut(SHIFT_DATA, SHIFT_CLK, MSBFIRST, address);

  digitalWrite(SHIFT_LATCH, LOW);
  digitalWrite(SHIFT_LATCH, HIGH);
  digitalWrite(SHIFT_LATCH, LOW);
}

byte readEEPROM(int address) {
  setAddress(address);

  digitalWrite(OUTPUT_ENABLE_AL, HIGH);
  digitalWrite(OUTPUT_ENABLE_AL, LOW);

  int pinCount = sizeof(dataPins) / sizeof(int);

  byte data = 0;
  for (int pin = pinCount; pin >= 0; pin -= 1) {
    data = (data << 1) + digitalRead(dataPins[pin]);
  }

  digitalWrite(OUTPUT_ENABLE_AL, HIGH);

  return data;
}

void writeEEPROM(int address, byte data) {
  setAddress(address);

  int pinCount = sizeof(dataPins) / sizeof(int);

  for (int pin = 0; pin < pinCount; pin += 1) {
    digitalWrite(dataPins[pin], data & 1);
    data = data >> 1;
  }

  digitalWrite(WRITE_ENABLE_AL, HIGH);
  digitalWrite(WRITE_ENABLE_AL, LOW);
  delayMicroseconds(1);
  digitalWrite(WRITE_ENABLE_AL, HIGH);

  delay(10);
}

void printContents(int start, int length) {
  setReadMode();

  for (int base = start; base < length; base += 16) {
    byte data[16];
    for (int offset = 0; offset <= 15; offset += 1) {
      data[offset] = readEEPROM(base + offset);
    }

    char buf[80];
    sprintf(buf, "%03x:  %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
            base, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
            data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);

    Serial.println(buf);
  }
}

void setup() {
  initUCode();

  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(SHIFT_CLK, OUTPUT);
  pinMode(SHIFT_LATCH, OUTPUT);
  pinMode(SHIFT_DATA, OUTPUT);

  // Active low
  digitalWrite(WRITE_ENABLE_AL, HIGH);
  pinMode(WRITE_ENABLE_AL, OUTPUT);

  // Active low
  digitalWrite(OUTPUT_ENABLE_AL, HIGH);
  pinMode(OUTPUT_ENABLE_AL, OUTPUT);

  Serial.begin(57600);

  setWriteMode();
  Serial.print("Programming microcodes");

  // Program the 8 high-order bits of microcode into the first 128 bytes of EEPROM
  for (int address = 0; address < 1024; address += 1) {
    int flags       = (address & 0b1100000000) >> 8;
    int byte_sel    = (address & 0b0010000000) >> 7;
    int instruction = (address & 0b0001111000) >> 3;
    int step        = (address & 0b0000000111);

    if (byte_sel) {
      writeEEPROM(address, ucode[flags][instruction][step]);
    } else {
      writeEEPROM(address, ucode[flags][instruction][step] >> 8);
    }

  }

  printContents(0, 1024);
}


void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
}
