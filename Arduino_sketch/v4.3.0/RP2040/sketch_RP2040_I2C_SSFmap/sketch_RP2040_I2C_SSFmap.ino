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
  WAIT, READ, WRITE
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
void write_byte2(uint32_t address, uint8_t data);
void write_byte16(uint32_t address, uint8_t dataA, uint8_t dataB);
void select_chip(chipType new_chip);
void Erase16BIT(void);
void ReadChipID(void);
//-----------------------------------//
void set_addressRead(uint32_t address);
void led_on(void);
void led_off(void);
void set_addressBank(uint32_t address);
void set_byteBank(uint32_t address, uint8_t data);
void BankInit(void);
void BankCheck(uint8_t Bank);
//----------------------------------//

chipType chip = NONE;
Modes mode = WAIT;
uint32_t start_address = 0x00000000;
uint32_t end_address   = 0x00000000;

#define BUF_LEN 8
uint8_t buf[BUF_LEN];
uint32_t ij = 0x00000000;
//----------//
uint8_t BK=6;
uint32_t Bank_address = 0x00200000;
//--^^^^^--//

void setup() {
  // Безопасная неблокирующая инициализация USB Serial (макс. 2 сек)
  Serial.begin(115200);
  uint32_t timeout = millis();
  while (!Serial && (millis() - timeout < 2000)) {
    tight_loop_contents();
  }

  // Явное назначение I2C (I2C0) для RP2040 
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
         //----------//
           BankInit();
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
          //----------//
           BankInit();  
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
        
  default: {
      if (Serial.available() > 0) {
        char incomingByte = Serial.read();
        switch (incomingByte) {
          case 'p': Serial.println("Arduino 29 Series programmer"); break;//ReadChipID(); // Функция считает ID и отправит 2 байта через Serial.write
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

//-------//
void set_address(uint32_t address) 
{  
  // Если адрес равен или превышает 4 МБ (0x200000 в словах), 
  // автоматически фиксируем биты A19 и A20 в единицы (маска 0x180000).
  // Все остальные младшие биты (включая A18) будут переключаться счетчиком сами.

  uint32_t target_address = address & 0x1FFFFF; // A21,A22,A23 == 0;  
  
  if (address >= Bank_address) 
  {
    Bank_address += 0x00080000; //Bank_address = 0x00200000 + 1MB: 0x00080000;
      BankCheck(BK);
  }

  if (address >= 0x200000) 
  {
    target_address |= 0x180000; //A19,A20 == 1;
  }

  digitalWrite(shiftLatchPin, LOW);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, (target_address >> 16) & 0xFF);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, (target_address >> 8) & 0xFF);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, target_address & 0xFF);
  digitalWrite(shiftLatchPin, HIGH);
}

void set_addressRead(uint32_t address) 
{  
  // Если адрес равен или превышает 4 МБ (0x200000 в словах). 
  // автоматически фиксируем биты A19 и A20 в единицы (маска 0x180000).
  // Все остальные младшие биты (включая A18) будут переключаться счетчиком сами.

  uint32_t target_address = address & 0x1FFFFF; // A21,A22,A23 == 0;  
  
  if (address >= Bank_address) 
  {
    Bank_address += 0x00080000;  //Bank_address = 0x00200000 + 1MB: 0x00080000;
      BankCheck(BK);
    read_mode();
      digitalWrite(chipEnable, LOW);
      digitalWrite(outputEnable, LOW);
  }

  if (address >= 0x200000) 
  {
    target_address |= 0x180000; //A19,A20 == 1;
  }

  digitalWrite(shiftLatchPin, LOW);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, (target_address >> 16) & 0xFF);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, (target_address >> 8) & 0xFF);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, target_address & 0xFF);
  digitalWrite(shiftLatchPin, HIGH);
}  
//-^^^^-//

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
  digitalWrite(WE, HIGH);
  digitalWrite(chipEnable, HIGH);  

  Serial.write('k');
}

void write_byte2(uint32_t address, uint8_t data) {
  set_address(address);
  set_data(data);
}

void ReadChipID() {
  digitalWrite(chipEnable, HIGH);
  digitalWrite(outputEnable, HIGH);
  digitalWrite(WE, HIGH);
  
  BankInit();  //write_mode();

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
  set_addressRead(0x0000);
  uint8_t mfgId = get_dataA(); //delayMicroseconds(1);

  // Цикл 2: Device ID - Первый байт (Адрес 0x0001)
  digitalWrite(chipEnable, HIGH); digitalWrite(chipEnable, LOW);
  set_addressRead(0x0001); 
  uint8_t cycle2 = get_dataA(); //delayMicroseconds(1);

  uint16_t finalDevId = cycle2;

  // Если второй цикл равен 0x7E, читаем третий цикл (например, по адресу 0x000E или сдвигу согласно даташиту)
  if (cycle2 == 0x7E) {
      digitalWrite(chipEnable, HIGH); digitalWrite(chipEnable, LOW);
      set_addressRead(0x000E); 
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

  BankInit(); //write_mode();

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
    set_addressRead(0x0000);
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
    yield(); // Чтобы не падал USB-стек RP2040
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

//----------------------------------------------------//----------------------------------------------------//
//----------------------------------------------------//----------------------------------------------------//
//BANCK SWICHING
void set_addressBank(uint32_t address) 
{  // Если адрес равен или превышает 4 МБ (0x200000 в словах), // автоматически фиксируем биты A19 и A20 в единицы (маска 0x180000).// Все остальные младшие биты (включая A18) будут переключаться счетчиком сами.
  
  digitalWrite(shiftLatchPin, LOW);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, (address >> 16) & 0xFF);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, (address >> 8) & 0xFF);
  shiftOut(shiftDataPin, shiftClockPin, MSBFIRST, address & 0xFF);
  digitalWrite(shiftLatchPin, HIGH);
}

void set_byteBank(uint32_t address, uint8_t data) 
{
  set_addressBank(address);
  set_data(data);
}


void BankInit() 
{ 
    Bank_address = 0x00200000;
    BK=6;
    BankCheck(BK);
}

void BankCheck(uint8_t Bank)
{
    digitalWrite(chipEnable, HIGH);
    digitalWrite(outputEnable, HIGH);
    digitalWrite(WE, HIGH);
write_mode();
    // AREA:6  //Запись значения 6 по адресу 0x50987E (двоичный: 1xx xxxx xxxx xxxx x111 1110)
 set_byteBank(0x50987E, Bank);    //delayMicroseconds(1);   //  0x50987E (A130FD/2)
    digitalWrite(PIN_TIME, LOW);    // Импульс #TIME и #LWR (опускаем и поднимаем)
    digitalWrite(PIN_LWR, LOW);      
    digitalWrite(PIN_TIME, HIGH);
    digitalWrite(PIN_LWR, HIGH);
    
    // AREA:7  //Запись значения 7 по адресу 0x50987F (двоичный: 1xx xxxx xxxx xxxx x111 1111)
 set_byteBank(0x50987F, Bank + 1); //delayMicroseconds(1);   //0x50987F  (A130FF/2)
    digitalWrite(PIN_TIME, LOW);   // Импульс #TIME и #LWR
    digitalWrite(PIN_LWR, LOW);      
    digitalWrite(PIN_TIME, HIGH);
    digitalWrite(PIN_LWR, HIGH);    
  BK += 2;  
//read_mode();
}