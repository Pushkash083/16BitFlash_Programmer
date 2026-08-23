#include <Wire.h>
#include <MCP23017.h>

#define MCP23017_ADDR 0x20
MCP23017 mcp = MCP23017(MCP23017_ADDR);

/* 74HC595 control (address lines) */
#define shiftDataPin  A0
#define shiftLatchPin A1
#define shiftClockPin A2

/* Chip control */
#define chipEnable   8
#define outputEnable 9
#define WE           A3

typedef enum chipType {
  NONE = 0,
  C16  = 1,
  C32  = 2,
  C64  = 3,
  C128 = 4,
  G3Mb = 5,
  C256 = 6,
  C512 = 7,
  MB32 = 8
} Chip;

typedef enum mode {
  WAIT,
  READ,
  WRITE,
  VOLTAGE
} Modes;

// Объявления функций
void write_mode(void);
void read_mode(void);
void set_address(uint32_t address);
uint8_t get_data(void);
uint8_t get_dataB(void);
void set_data(uint8_t data);
uint8_t read_byte(uint32_t address);
uint8_t read_byteB(uint32_t address);
void write_byte(uint32_t address, uint8_t data);
void write_byte2(uint32_t address, uint8_t data);
void write_byte16(uint32_t address, uint8_t dataA, uint8_t dataB);
void select_chip(chipType new_chip);
void Erase16BIT(void);
void SDProtectD(void);
void InitPro(void);
void writ1b(void);

chipType chip = NONE;
Modes mode = WAIT;
uint32_t start_address = 0x00000000;
uint32_t end_address   = 0x00000000;

#define BUF_LEN 8
uint8_t buf[BUF_LEN];
uint32_t ij = 0x00000000;

void setup() {
  Wire.begin(); // Стандартный I2C для Arduino (SDA = A4, SCL = A5)
  Wire.setClock(400000);
  mcp.init();

  pinMode(shiftLatchPin, OUTPUT);
  pinMode(shiftClockPin, OUTPUT);
  pinMode(shiftDataPin,  OUTPUT);
  pinMode(WE,           OUTPUT);
  pinMode(chipEnable,   OUTPUT);
  pinMode(outputEnable, OUTPUT);

  digitalWrite(WE, HIGH);
  digitalWrite(outputEnable, LOW);
  digitalWrite(chipEnable, HIGH);

  set_address(0x00000000);
  //read_mode();

  Serial.begin(115200);
  Serial.println("Arduino 29 Series programmer");
  read_mode();
}

void loop() {
  switch (mode) {
    case READ: {
      if (chip == NONE) {
        mode = WAIT;
        break;
      }

      read_mode();
      digitalWrite(WE, HIGH);
      digitalWrite(chipEnable, LOW);
      digitalWrite(outputEnable, LOW);

      for (uint32_t i = start_address; i <= end_address; i++) {
        set_address(i);
        uint8_t dataB = get_data();   // Port A - Младший байт (LSB)
        uint8_t dataA = get_dataB();  // Port B - Старший байт (MSB)

        Serial.write(dataA);          // Отправка в правильном порядке
        Serial.write(dataB);

        if ((i & 0x3F) == 0) {
          Serial.flush();
        }
      }

      digitalWrite(chipEnable, HIGH);
      mode = WAIT;
      break;
    }

    case WRITE: {
      if (chip == NONE) {
        mode = WAIT;
        break;
      }

      write_mode();
      digitalWrite(chipEnable, HIGH);
      digitalWrite(outputEnable, HIGH);
      digitalWrite(WE, HIGH);

      ij = 0;
      while (ij <= end_address) {
        if (Serial.available() >= 2) {
          uint8_t dataA = Serial.read(); // Младший байт
          uint8_t dataB = Serial.read(); // Старший байт
          if (dataA != 0xFF || dataB != 0xFF) {  
             write_byte16(ij, dataA, dataB);
          } else {
             Serial.write('k'); // Пропуск 0xFF (стереть не нужно), даем подтверждение
          }
          ij++;
        }
      }

      digitalWrite(WE, HIGH);
      digitalWrite(chipEnable, HIGH);
      digitalWrite(outputEnable, LOW);
      read_mode();

      mode = WAIT;
      break;
    }

    case VOLTAGE:
      mode = WAIT;
      break;

    default: {
      if (Serial.available() > 0) {
        char incomingByte = Serial.read();
        switch (incomingByte) {
          case 'p': Serial.println("Arduino 29 Series programmer"); break;
          case 'r': mode = READ; read_mode(); break;
          case 'w': mode = WRITE; write_mode(); ij = 0; break;
          case 'v': mode = VOLTAGE; break;
          case 'g': Erase16BIT(); break;
          case 'h': SDProtectD(); break;
          case 'n': writ1b(); break;
          case 'a': select_chip(C16); break;
          case 'b': select_chip(C32); break;
          case 'c': select_chip(C64); break;
          case 'd': select_chip(C128); break;
          case 'q': select_chip(G3Mb); break;
          case 'e': select_chip(C256); break;
          case 'f': select_chip(C512); break;
          case 'z': {
            // Ждем прихода 4 байт с количеством слов от приложения Qt
            while (Serial.available() < 4) {}
            
            uint32_t wordsCount = 0;
            wordsCount |= ((uint32_t)Serial.read() << 24);
            wordsCount |= ((uint32_t)Serial.read() << 16);
            wordsCount |= ((uint32_t)Serial.read() << 8);
            wordsCount |= (uint32_t)Serial.read();

            // Коррекция адреса
            if (wordsCount > 0) {
              end_address = wordsCount - 1;
            } else {
              end_address = 0;
            }
            
            chip = MB32;
            break;
          }
        }
      }
      break;
    }
  }
}

void select_chip(chipType new_chip) {
  chip = new_chip;
  switch (new_chip) {
    case C16:  end_address = 0x00003FFF; break;
    case C32:  end_address = 0x0003FFFF; break;
    case C64:  end_address = 0x0007FFFF; break;
    case C128: end_address = 0x000FFFFF; break;
    case G3Mb: end_address = 0x0017FFFF; break;
    case C256: end_address = 0x001FFFFF; break;
    case C512: end_address = 0x003FFFFF; break;
    case MB32: end_address = 0x00FFFFFF; break;
    default:   chip = NONE; end_address = 0x00000000; break;
  }
}

void write_mode(void) {
  mcp.portMode(MCP23017Port::B, 0x00);
  mcp.portMode(MCP23017Port::A, 0x00);
  mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);
  mcp.writeRegister(MCP23017Register::GPIO_B, 0x00);
}

void read_mode(void) {
  mcp.portMode(MCP23017Port::A, 0xFF, INPUT_PULLUP);
  mcp.portMode(MCP23017Port::B, 0xFF, INPUT_PULLUP);
  mcp.writeRegister(MCP23017Register::GPIO_A, 0x00);
  mcp.writeRegister(MCP23017Register::GPIO_B, 0x00);
}

void set_address(uint32_t address) {
  digitalWrite(shiftLatchPin, LOW);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, (address >> 16) & 0xFF);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, (address >> 8) & 0xFF);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, address & 0xFF);
  digitalWrite(shiftLatchPin, HIGH);
}

uint8_t get_data(void) {
  return mcp.readPort(MCP23017Port::A);
}

uint8_t get_dataB(void) {
  return mcp.readPort(MCP23017Port::B);
}

void set_data(uint8_t data) {
  mcp.writePort(MCP23017Port::A, data);
  mcp.writePort(MCP23017Port::B, 0x00);
}

uint8_t read_byte(uint32_t address) {
  set_address(address);
  return get_data();
}

uint8_t read_byteB(uint32_t address) {
  set_address(address);
  return get_dataB();
}

void write_byte16(uint32_t address, uint8_t dataA, uint8_t dataB) {
  digitalWrite(chipEnable, HIGH);
  digitalWrite(outputEnable, HIGH);
  digitalWrite(WE, HIGH);

  write_byte2(0x555, 0xAA);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);

  write_byte2(0x2AA, 0x55);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);

  write_byte2(0x555, 0xA0);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);

  set_address(address);
  mcp.writePort(MCP23017Port::A, dataB); // Port A = LSB
  mcp.writePort(MCP23017Port::B, dataA); // Port B = MSB

  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  delayMicroseconds(10);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);
  delayMicroseconds(1);

  Serial.write('k'); // Ответ стороне Qt для синхронизации
}

void write_byte2(uint32_t address, uint8_t data) {
  set_address(address);
  set_data(data);
}

void write_byte(uint32_t address, uint8_t data) {
  set_address(address);
  set_data(data);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  delayMicroseconds(1);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);
}

void writ1b() {
  InitPro();
  write_byte(0x0003728C, 0x94);
  Serial.println(read_byte(0x0003728C));
}

void InitPro() {
  digitalWrite(chipEnable, HIGH);
  digitalWrite(outputEnable, HIGH);
  digitalWrite(WE, HIGH);

  write_byte2(0x555, 0xAA);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);

  write_byte2(0x2AA, 0x55);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);

  write_byte2(0x555, 0xA0);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);
  digitalWrite(outputEnable, HIGH);
}

void SDProtectD() {
  digitalWrite(chipEnable, LOW);
  digitalWrite(outputEnable, HIGH);
  digitalWrite(WE, HIGH);

  write_byte2(0x555, 0xAA);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);

  write_byte2(0x2AA, 0x55);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);

  write_byte2(0x555, 0x80);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);

  write_byte2(0x555, 0xAA);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);

  write_byte2(0x2AA, 0x55);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);

  write_byte2(0x555, 0x10);
  digitalWrite(WE, LOW);
  digitalWrite(WE, HIGH);
}

void Erase16BIT() {
  digitalWrite(chipEnable, HIGH);
  digitalWrite(outputEnable, HIGH);
  digitalWrite(WE, HIGH);

  write_mode();

  set_address(0x555);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  mcp.writePort(MCP23017Port::A, 0xAA);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);

  set_address(0x2AA);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  mcp.writePort(MCP23017Port::A, 0x55);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);

  set_address(0x555);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  mcp.writePort(MCP23017Port::A, 0x80);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);

  set_address(0x555);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  mcp.writePort(MCP23017Port::A, 0xAA);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);

  set_address(0x2AA);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  mcp.writePort(MCP23017Port::A, 0x55);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);

  set_address(0x555);
  digitalWrite(chipEnable, LOW);
  digitalWrite(WE, LOW);
  mcp.writePort(MCP23017Port::A, 0x10);
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);

  read_mode();
}