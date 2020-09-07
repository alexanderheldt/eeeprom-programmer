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

void printContents() {
  setReadMode();

  for (int base = 0; base <= 255; base += 16) {
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

  // Bit patterns for the digits 0..9
  byte digits[] = { 0x7e, 0x30, 0x6d, 0x79, 0x33, 0x5b, 0x5f, 0x70, 0x7f, 0x7b };

  setWriteMode();
  Serial.println("Programming ones place");
  for (int value = 0; value <= 255; value += 1) {
    writeEEPROM(value, digits[value % 10]);
  }

  Serial.println("Programming tens place");
  for (int value = 0; value <= 255; value += 1) {
    writeEEPROM(value + 256, digits[(value / 10) % 10]);
  }

  Serial.println("Programming hundreds place");
  for (int value = 0; value <= 255; value += 1) {
    writeEEPROM(value + 512, digits[(value / 100) % 10]);
  }

  Serial.println("Programming sign");
  for (int value = 0; value <= 255; value += 1) {
    writeEEPROM(value + 768, 0);
  }

  printContents();
}

void loop() {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(1000);
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
}
