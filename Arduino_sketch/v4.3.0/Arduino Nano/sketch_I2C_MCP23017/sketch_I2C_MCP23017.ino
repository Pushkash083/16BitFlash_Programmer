// A0  -> SER
// A1  -> L_CLK
// A2  -> S_CLK
// A3  -> WE
// A4  -> A4(SDA)
// A5  -> A5(SCL)
// D13 -> SSCK   |<-NC
// D12 -> SMISO  |<-NC
// D11 -> SMOSI  |<-NC
// D10 -> SCS    |<-NC
// D9  -> OE
// D8  -> CE
// D7  -> W/R(LED)
// D6  -> LWR
// D5  -> TIME
// RESET  -> RST

#include <Wire.h>
#include <MCP23017.h>

#define MCP23017_ADDR 0x20
MCP23017 mcp = MCP23017(MCP23017_ADDR);

/* 74HC595 control (address lines) */
#define shiftDataPin  A0  //A0
#define shiftLatchPin A1  //A1
#define shiftClockPin A2  //A2

/* Chip control */
#define chipEnable   8    //D8  ((12))
#define outputEnable 9    //D9  ((11))
#define WE           A3   //A3

#define LED_BUSY_PIN 7    // D7
#define PIN_TIME     5    // D5
#define PIN_LWR      6    // D6

typedef enum chipType {
  NONE = 0, KB32 = 1, KB512 = 2, MB1 = 3, MB2 = 4,
  MB3 = 5, MB4 = 6, MB8 = 7, MB16 = 8, MB32 = 9
} Chip;

typedef enum mode {
  WAIT, READ, WRITE
} Modes;

// Объявления функций
void write_mode(void);
void read_mode(void);
void set_address(uint32_t address);
uint8_t get_dataA(void);
uint8_t get_dataB(void);
void set_data(uint8_t data);
uint8_t read_byte(uint32_t address);
uint8_t read_byteB(uint32_t address);
void write_byte2(uint32_t address, uint8_t data);
void write_byte16(uint32_t address, uint8_t dataA, uint8_t dataB);
void select_chip(chipType new_chip);
void Erase16BIT(void);
void ReadChipID(void);
void led_on(void);
void led_off(void);

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
      led_on();

      for (uint32_t i = start_address; i <= end_address; i++) {
        set_address(i);
        uint8_t dataB = get_dataA();   // Port A - Младший байт (LSB)
        uint8_t dataA = get_dataB();  // Port B - Старший байт (MSB)

        Serial.write(dataA);          // Отправка в правильном порядке
        Serial.write(dataB);

        if ((i & 0x3F) == 0) {
          Serial.flush();
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
             Serial.write('k'); // Пропуск 0xFF (стереть не нужно), даем подтверждение
          }
          ij++;
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

    default: {
      if (Serial.available() > 0) {
        char incomingByte = Serial.read();
        switch (incomingByte) {
          case 'p': Serial.println("Arduino 29 Series programmer"); break;
          case 'r': mode = READ; read_mode(); break;
          case 'w': mode = WRITE; write_mode(); ij = 0; break;          
          case 'g': Erase16BIT(); break;          
          case 'i': ReadChipID(); break;       
          case 'a': select_chip(KB32); break;
          case 'b': select_chip(KB512); break;
          case 'c': select_chip(MB1); break;
          case 'd': select_chip(MB2); break;
          case 'q': select_chip(MB3); break;
          case 'e': select_chip(MB4); break;
          case 'f': select_chip(MB8); break;
          case 'z': {
            uint32_t timeout = millis();
            while (Serial.available() < 4 && (millis() - timeout < 1000)) {}
            
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
              chip = KB32;
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

  Serial.write('k'); // Ответ стороне Qt для синхронизации
}

void write_byte2(uint32_t address, uint8_t data) {
  set_address(address);
  set_data(data);
}

void ReadChipID() {
  digitalWrite(chipEnable, HIGH);
  digitalWrite(outputEnable, HIGH);
  digitalWrite(WE, HIGH);
  
  write_mode();
   delay(50);

  // Команда 0x90
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

  write_byte2(0x555, 0x90);
     digitalWrite(chipEnable, LOW); 
     digitalWrite(WE, LOW); 
     digitalWrite(WE, HIGH); 
     digitalWrite(chipEnable, HIGH);

  read_mode();
  digitalWrite(chipEnable, LOW);
  digitalWrite(outputEnable, LOW);

  // Читаем ID
  // Цикл 1: Manufacturer ID (Адрес 0x0000)
  set_address(0x0000);
  uint8_t mfgId = get_dataA(); //delayMicroseconds(1);

  // Цикл 2: Device ID - Первый байт (Адрес 0x0001)
    digitalWrite(chipEnable, HIGH); 
    digitalWrite(chipEnable, LOW);
  set_address(0x0001); 
  uint8_t cycle2 = get_dataA(); //delayMicroseconds(1);

  uint16_t finalDevId = cycle2;

  // Если второй цикл равен 0x7E, читаем третий цикл (например, по адресу 0x000E или сдвигу согласно даташиту)
  if (cycle2 == 0x7E) {
        digitalWrite(chipEnable, HIGH); 
        digitalWrite(chipEnable, LOW);
      set_address(0x000E); 
      uint8_t cycle3 = get_dataA(); //delayMicroseconds(1);
      
      // Склеиваем в 16-битный CID: старший байт 0x7E, младший — cycle3 (например, 0x7E1A)
      finalDevId = ((uint16_t)0x7E << 8) | cycle3;
  } else {
      // Для обычных чипов старший байт будет 0x00, младший — cycle2
      finalDevId = cycle2; 
  }

  digitalWrite(chipEnable, HIGH);
  digitalWrite(outputEnable, HIGH);    

  // Сброс чипа (команда 0xF0)
  write_mode();
  write_byte2(0x0000, 0xF0);
     digitalWrite(chipEnable, LOW); 
     digitalWrite(WE, LOW); 
     digitalWrite(WE, HIGH); 
     digitalWrite(chipEnable, HIGH);
  read_mode();
    
  // Отправляем на ПК: 1 байт MID и 2 байта CID (старший, затем младший)
  Serial.write(mfgId);
  Serial.write((uint8_t)(finalDevId >> 8));
  Serial.write((uint8_t)(finalDevId & 0xFF));
}

void Erase16BIT() 
{
  uint32_t Timeout = millis();
  uint32_t blinkTimer = millis(); //blinkTimer
  uint8_t status1 = get_dataA();
  bool LED = false;

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

// ---  ЦИКЛ ОПРОСА  ---
  read_mode(); // Переводим пины в режим чтения для опроса статуса кристалла
  delay(200);

  while (millis() - Timeout < 90000) // Таймаут 90 секунд
  {
    // Читаем статус по адресу 0x0000 
    set_address(0x0000);
       digitalWrite(chipEnable, LOW);
       digitalWrite(outputEnable, LOW);    
    status1 = get_dataA();  delayMicroseconds(5);
       digitalWrite(chipEnable, HIGH);
       digitalWrite(outputEnable, HIGH);
    
    // Проверяем DQ7 : статус 1
    if (status1 & 0x80) 
    {
        break; // Стирание реально завершено!
    }

   // Мигаем светодиодом каждые 150мс
    if (millis() - blinkTimer >= 150) 
    {
      blinkTimer = millis();
      LED = !LED;
      digitalWrite(LED_BUSY_PIN, LED);
    }    
  }    
    led_off();
    read_mode();
     Serial.write(status1);  // --- ОТПРАВКА БАЙТА СТАТУСА В QT --- 
}

void led_on() {
    digitalWrite(LED_BUSY_PIN, HIGH);
}

void led_off() {
    digitalWrite(LED_BUSY_PIN, LOW);
}