# 16BitFlash Programmer (SEGA Mega Drive / Genesis)

🌐 **Limbă / Язык:** [Русский](README.md) | Română

![Version](https://img.shields.io/badge/version-3.0.0-blue)
![Platform](https://img.shields.io/badge/platform-RP2040%20%7C%20Arduino%20Nano%20%7C%20ESP8266%20%7C%20Qt%206-green)
![License](https://img.shields.io/badge/license-MIT-orange)

> 💡 **Istoria proiectului:** Proiectul a fost conceput inițial ca un simplu experiment pentru a înțelege funcționarea portului COM, protocolului UART și transferului de date între PC și dispozitive externe, dar a evoluat într-un programator complet.

Programator bazat pe microcontrollere **Raspberry Pi Pico (RP2040)** sau **Arduino Nano** și software de control în **Qt 6 C++** pentru citirea, scrierea, verificarea și ștergerea cipurilor Flash pe 16 biți (seriile **MX29LV, AM29F, ES29LV, HY29F, S29AL, S29GL** etc.) și a cartușelor **SEGA Mega Drive / Genesis**.

---

## 🚀 Noutăți în versiunea v3.0.0

* **Trecerea la RP2040 (Raspberry Pi Pico):** Microcontroller ARM Cortex-M0+ pe 32 biți.
* **Actualizare software PC (Qt GUI):** Aplicația desktop a primit suport complet și optimizări pentru protocolul de comunicare cu microcontrollerul **RP2040**.
* **Creșterea vitezei:** Timpul de scriere pentru **1 MB** a scăzut de la **~30 minute** (Arduino Nano) la **~5 minute** (RP2040). Scrierea fișierelor mari de **15 MB** durează acum doar **~1 oră și 10 minute** în loc de **~8 ore și 20 minute** (Arduino Nano).
* **Placă de circuit nouă (PCB v3.0.0):** PCB cu două straturi, soclu dublu (Pico + Nano), indicare LED pentru operațiuni și suport pentru expansoare rapide **MCP23S17 (SPI)** / **MCP23017 (I2C)**.

---

## 🖥 Interfață și Funcții (Qt GUI)

![Interface](Interface.png)

* **Monitorizare:** Cronometru LCD și fereastră pentru jurnale (loguri).
* **Selecție ROM:** Dimensiuni fixe de la 512 KB până la 16 MB, `TEST 32KB` (verificare rapidă) și modul inteligent `Auto ROM Select`.
* **Vizualizator Hex:** Vizualizare integrată a memoriei (`Show buffer`) pe 16 coloane.
* **Operațiuni principale:** Citire, Scriere, Ștergere, Verificare.

---

## 🌟 Funcționalități Cheie

* **Auto ROM Select Inteligent:** Detectează automat dimensiunea fișierului. Dacă dimensiunea se potrivește cu un preset standard, selectează opțiunea corespunzătoare; dacă nu, transmite microcontrollerului adresa finală exactă a fișierului de orice dimensiune. Pentru lucrul cu volume de peste 4 MB (8 MB și 16 MB) este necesară lipirea liniilor de adresă $A21/A22$ ale cipului de memorie.
* **Integrare Hardware:** Magistrală de date pe 16 biți prin MCP23017 (I2C) / MCP23S17 (SPI), magistrală de adrese prin cascadă de 74HC595.
* **Byte Swapping Bidirecțional:** Programatorul inversează automat octeții atât la citire, cât și la scriere. Fișierele `.bin`, `.gen`, `.md` sunt gata direct de utilizare, fără utilitare terțe.
* **Testare Rapidă:** Modul `TEST 32KB` permite verificarea rapidă a unei porțiuni de date și a funcționării magistralei.
* **Verificare ROM:** Verificarea datelor după scriere.
* **Protecție Sistem:** Blocarea modului Standby (Sleep) al sistemului de operare în timpul lucrului cu Flash.

---

## 🔌 Schema de Conectare

### 1. Magistrală Adrese (74HC595)
| Semnal | RP2040 (Pico) | Arduino Nano | ESP8266 |
| :--- | :--- | :--- | :--- |
| **SER (Data)** | GP2 | A0 | D4 |
| **L_CLK (Latch)** | GP3 | A1 | D5 |
| **S_CLK (Clock)** | GP6 | A2 | D3 |

### 2. Linii Control Flash, Indicare și Control Bănci
| Semnal | RP2040 (Pico) | Arduino Nano | ESP8266 | Descriere |
| :--- | :--- | :--- | :--- | :--- |
| **#WE** | GP7 | A3 | D7 | Write Enable (LOW) |
| **#CE** | GP8 | D8 | D0 | Chip Enable (LOW) |
| **#OE** | GP9 | D9 | D6 | Output Enable (LOW) |
| **LED (W/R)** | GP20 | D7 | D2 | Indicator LED proces citire/scriere |
| **#TIME** | GP21 | D5 | — | Rezervat|
| **#LWR** | GP22 | D6 | — | Rezervat|

### 3. Magistrală Date (MCP23S17 / MCP23017)

#### MCP23017 (Mod I2C)
| Semnal MCP23017 | RP2040 (Pico) | Arduino Nano | ESP8266 | Descriere |
| :--- | :--- | :--- | :--- | :--- |
| **SDA** | GP4 | A4 | D1 | Linie date I2C |
| **SCL** | GP5 | A5 | D2 | Linie ceas I2C |

#### MCP23S17 (Mod SPI — Recomandat v3.0.0)
| Semnal MCP23S17 | RP2040 (Pico) | Arduino Nano | Descriere |
| :--- | :--- | :--- | :--- |
| **CS** | GP17 | D10 | Chip Select |
| **SCK** | GP18 | D13 | Clock SPI |
| **MOSI (SI)** | GP19 | D11 | Master Out Slave In |
| **MISO (SO)** | GP16 | D12 | Master In Slave Out |

---

## 💾 Cipuri Suportate

* **Seria MX29LV / MX29F:** MX29LV400, MX29LV800, MX29LV160, MX29LV320, MX29LV640, MX29F1615 etc.
* **Seria Spansion / Cypress:** S29AL016M, S29GL256S și echivalente.
* **Seria AM29 / Altele:** AM29F800, ES29LV800D, HY29F800TT, M29W160ET, EN29LV160A.

---

## 🛠 Hardware

În repozitoriu sunt disponibile fișierele pentru asamblarea dispozitivului:
* **[Schema programatorului (PDF)](Hardware/)** — Schema electrică principială a programatorului.
* **[Gerber](Hardware)** — Fișiere Gerber gata pentru comanda fabricării plăcii de circuit (PCB).

---

## 📦 Lansări și Software

**Software pentru Windows:** În secțiunea **[Releases](../../releases/latest)** este disponibilă pentru descărcare aplicația de control, precum și codul sursă al proiectului.

---

## 👨‍💻 Autor

* **Dezvoltator:** Pushkash
* **Asistent AI:** Gemini
