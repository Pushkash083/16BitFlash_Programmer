// =========================================================================
// RP2040 16-Bit Flash Programmer (29 Series) - I2C Version (MCP23017)
//  WE        = GP7
//  CE        = GP8
//  OE        = GP9
//  W/R (LED) = GP20
//  TIME      = GP21
//  LWR       = GP22
//==74HC595 (Адрес): 
//  DataPin =  GP2
//  LatchPin = GP3
//  ClockPin = GP6
//==MCP23017 (I2C):
//->SDA = GP4
//->SCL = GP5
// =========================================================================

#include <Wire.h>
#include <MCP23017.h>

#define MCP23017_ADDR 0x20
MCP23017 mcp = MCP23017(MCP23017_ADDR);

/* 74HC595 control (address lines) */
#define shiftDataPin  2 // GP2
#define shiftLatchPin 3 // GP3
#define shiftClockPin 6 // GP6

/* Chip control */
#define WE           7   //GP7
#define chipEnable   8   //GP8
#define outputEnable 9   //GP9

#define LED_BUSY_PIN 20   // GP20
#define PIN_TIME  21      // GP21
#define PIN_LWR   22      // GP22

typedef enum chipType {
  NONE = 0, KB32 = 1, KB512 = 2, MB1 = 3, MB2 = 4,
  MB3 = 5, MB4 = 6, MB8 = 7, MB16 = 8, MB32 = 9
} Chip;

typedef enum mode {
  WAIT, READ, WRITE, SerComm
} Modes;

// Прототипы функций
void write_mode(void);
void read_mode(void);
void set_address(uint32_t address);
uint8_t get_dataA(void);
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
  // Безопасная неблокирующая инициализация USB Serial (макс. 2 сек)
  Serial.begin(115200);
  uint32_t timeout = millis();
  while (!Serial && (millis() - timeout < 2000)) {
    tight_loop_contents();
  }

  // Явное назначение I2C (I2C0) для RP2040 (GP4 -> SDA / A4, GP5 -> SCL / A5)
  Wire.setSDA(4);  //SDA = GP4
  Wire.setSCL(5);  //SCL = GP5
  Wire.begin();
  Wire.setClock(400000);
  
  mcp.init();

  // Настройка выходов 74HC595 и сигналов Flash
  pinMode(shiftLatchPin, OUTPUT);
  pinMode(shiftClockPin, OUTPUT);
  pinMode(shiftDataPin,  OUTPUT);
  pinMode(WE,           OUTPUT);
  pinMode(chipEnable,   OUTPUT);
  pinMode(outputEnable, OUTPUT);
  pinMode(LED_BUSY_PIN, OUTPUT);
  pinMode(PIN_TIME, OUTPUT);
  pinMode(PIN_LWR, OUTPUT);

  digitalWrite(WE, HIGH);
  digitalWrite(outputEnable, LOW);
  digitalWrite(chipEnable, HIGH);

  digitalWrite(LED_BUSY_PIN, LOW);
  digitalWrite(PIN_TIME, HIGH);
  digitalWrite(PIN_LWR, HIGH);

  set_address(0x00000000);
  read_mode();

  //Очищаем буфер от старого стартового мусора
  while (Serial.available() > 0) {
    Serial.read();
    yield(); // Освежаем USB-стек RP2040, чтобы COM-порт не отваливался
  }

  // Ожидание handshake от Qt-клиента (команда 'p')
  bool connected = false;
  while (!connected) {
    if (Serial.available() > 0) {
      char incomingByte = Serial.read();
      if (incomingByte == 'p') {
        Serial.println("Arduino 29 Series programmer");
        connected = true;
      }
    }
    yield(); // Освежаем USB-стек RP2040, чтобы COM-порт не отваливался
  }
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
      led_on();

      for (uint32_t i = start_address; i <= end_address; i++) {
        set_address(i);
        uint8_t dataB = get_dataA();   // Port A - Младший байт (LSB)
        uint8_t dataA = get_dataB();  // Port B - Старший байт (MSB)

        Serial.write(dataA);          // Отправка в Qt
        Serial.write(dataB);

        if ((i & 0x3F) == 0) {
          Serial.flush();
          yield(); // Освежаем USB-стек RP2040
        }
      }

      digitalWrite(chipEnable, HIGH);
      led_off();
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
      led_on();

      ij = 0;
      while (ij <= end_address) {
        if (Serial.available() >= 2) {
          uint8_t dataA = Serial.read(); // Младший байт
          uint8_t dataB = Serial.read(); // Старший байт
          if (dataA != 0xFF || dataB != 0xFF) {  
             write_byte16(ij, dataA, dataB);
          } else {
             Serial.write('k');
          }
          ij++;
        } else {
          yield(); // Предотвращаем зависание при ожидании байт
        }
      }

      digitalWrite(WE, HIGH);
      digitalWrite(chipEnable, HIGH);
      digitalWrite(outputEnable, LOW);
      read_mode();
      led_off();

      mode = WAIT;
      break;
    }

    case SerComm:
      mode = WAIT;
      break;

    default: {
      if (Serial.available() > 0) {
        char incomingByte = Serial.read();
        switch (incomingByte) {
          case 'p': Serial.println("Arduino 29 Series programmer"); break;
          case 'r': mode = READ; read_mode(); break;
          case 'w': mode = WRITE; write_mode(); ij = 0; break;
          case 'v': mode = SerComm; break;
          case 'g': Erase16BIT(); break;
          case 'h': SDProtectD(); break;
          case 'n': writ1b(); break;
          case 'a': select_chip(KB32); break;
          case 'b': select_chip(KB512); break;
          case 'c': select_chip(MB1); break;
          case 'd': select_chip(MB2); break;
          case 'q': select_chip(MB3); break;
          case 'e': select_chip(MB4); break;
          case 'f': select_chip(MB8); break;
          case 'z': {
            uint32_t timeout = millis();
            while (Serial.available() < 4 && (millis() - timeout < 1000)) {
              yield();
            }
            
            if (Serial.available() >= 4) {
              uint32_t wordsCount = 0;
              wordsCount |= ((uint32_t)Serial.read() << 24);
              wordsCount |= ((uint32_t)Serial.read() << 16);
              wordsCount |= ((uint32_t)Serial.read() << 8);
              wordsCount |= (uint32_t)Serial.read();

              if (wordsCount > 0) {
                end_address = wordsCount - 1;
              } else {
                end_address = 0;
              }
              chip = MB4;
            }
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
    case KB32:  end_address = 0x00003FFF; break;
    case KB512: end_address = 0x0003FFFF; break;
    case MB1:   end_address = 0x0007FFFF; break;
    case MB2:   end_address = 0x000FFFFF; break;
    case MB3:   end_address = 0x0017FFFF; break;
    case MB4:   end_address = 0x001FFFFF; break;
    case MB8:   end_address = 0x003FFFFF; break;
    case MB16:  end_address = 0x007FFFFF; break;
    case MB32:  end_address = 0x00FFFFFF; break;
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

uint8_t get_dataA(void) {
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
  return get_dataA();
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

  Serial.write('k');
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

void led_on() {
    digitalWrite(LED_BUSY_PIN, HIGH);
}

void led_off() {
    digitalWrite(LED_BUSY_PIN, LOW);
}

// Дополнительно можно сделать функцию мигания при ошибке:
void led_error_blink() {
    for (int i = 0; i < 5; i++) {
        led_on();
        delay(100);
        led_off();
        delay(100);
    }
}