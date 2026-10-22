# 16BitFlash Programmer (SEGA Mega Drive / Genesis)

🌐 **Limba:** [Русский](README.md) | Română | [English](README_EN.md)

![Version](https://img.shields.io/badge/version-v4.3.0-blue)
![Platform](https://img.shields.io/badge/platform-RP2040%20%7C%20Arduino%20Nano%20%7C%20Qt%206-green)
![License](https://img.shields.io/badge/license-MIT-orange)

> 💡 **Istoricul proiectului:** Proiectul a început inițial ca un simplu experiment pentru a studia portul COM, protocolul UART și transferul de date între un PC și dispozitive externe, dar a evoluat într-un programator profesional complet.

Un programator bazat pe microcontrolerul **Raspberry Pi Pico (RP2040) sau Arduino Nano** și software de control în **Qt 6 C++** pentru **Citire, Scriere, Ștergere, Verificare și Auto-ID:** memorii Flash paralele pe 16 biți (seriile **MX29LV, AM29LV, S29GL** etc.) și cartușe **SEGA Mega Drive / Genesis**.

---

## 🚀 Ce este nou în versiunea v4.3.0

Versiunea **v4.3.0** încorporează toate funcționalitățile testate din versiunile anterioare și aduce actualizări majore pentru compatibilitatea hardware și flexibilitatea configurării:

### 1. Butonul „Chip ID” (Detectare automată a cipului)
* Funcție implementată pentru detectarea automată a cipului de memorie pe baza identificatorilor săi unici (MID și CID), citiți prin microcontrolerul RP2040 sau Arduino.
* Pe interfața principală au fost adăugate etichete informative (`Chip Information`) care afișează numele cipului detectat, ID-ul acestuia și capacitatea reală de memorie.
* La deschiderea unui fișier mai mare, selecția în program este permisă, dar bufferul și setările sunt trunchiate automat la dimensiunea fizică a cipului.

### 2. Baza de date externă pentru cipuri (`chips.txt`)
* S-a adăugat suport pentru un fișier de configurare extern **`chips.txt`**, aflat lângă executabilul programului. Din acesta, aplicația citește automat denumirile, producătorii (`MID`), ID-ul dispozitivului (`CID`) și dimensiunile memoriilor Flash.
* **Autogenerarea șablonului:** Dacă fișierul lipsește, programul îl va crea automat cu un set inițial de cipuri și comentarii. Acum puteți adăuga manual cipuri noi într-un fișier text simplu, fără a fi nevoie să recompilați codul sursă.

### 3. Monitorizarea hardware a ștergerii prin bitul `DQ7`
* S-au eliminat întârzierile fixe: procesul de ștergere este acum gestionat prin starea bitului **`DQ7`**, însoțit de clipirea vizuală a LED-ului de pe placa RP2040 sau Arduino.
* În aplicația PC s-a implementat o intercepție asincronă sigură a stării de pregătire prin semnale cu temporizator de protecție, menținând interfața complet responsivă în timpul ștergerii.

---

## 📜 Arhiva istoricului versiunilor (Changelog History)

<details>
<summary><b>🔽 Apăsați aici pentru a vizualiza versiunile anterioare</b></summary>

### Versiunea v3.1.1
* **Qt 6 GUI:** S-a adăugat posibilitatea de a selecta și lucra cu imagini ROM de până la 32 MB direct în interfața programului.
* **Suport RP2040:** Schițe noi adăugate pentru Raspberry Pi Pico (RP2040) cu suport pentru Sega Bank Switching (SSF2 Mapper).
* **Citire și scriere (SSF2):** Suport stabil implementat pentru citirea și scrierea imaginilor ROM de până la 32 MB prin mapperul SSF2 pe hardware dedicat, fără modificări de lipire (este necesar un cartuș cu mapper SSF2 încorporat).

### Versiunea v3.1.0
* **Actualizarea interfeței software (Qt GUI):** UI îmbunătățit, gestionare optimizată a dimensiunilor ROM printr-o listă derulantă (`QComboBox`).
* **Optimizarea protocolului de comunicație:** Aplicația desktop a fost adaptată și optimizată pentru ratele ridicate de transfer ale **RP2040**.

### Versiunea v3.0.0
* **Trecerea la RP2040 (Raspberry Pi Pico):** Microcontroler pe 32 de biți ARM Cortex-M0+.
* **Actualizarea software-ului PC (Qt GUI):** S-a adăugat suport complet și s-a optimizat protocolul de schimb de date pentru microcontrolerul **RP2040**.
* **Creșterea vitezei:** Timpul de scriere pentru **1 MB** a scăzut de la **~30 minute** (pe Arduino Nano) la **~5 minute** (pe RP2040). Scrierea unor dump-uri masive de **15 MB** durează doar **~1 oră și 10 minute** în loc de **~8 ore și 20 minute** (pe Arduino Nano).
* **Placă de circuit imprimat actualizată (PCB v3.0.0):** PCB cu două straturi, cu amprentă dublă (Pico + Nano), indicatori LED pentru operațiuni și suport pentru expandere rapide **MCP23S17 (SPI)** / **MCP23017 (I2C)**.

### Versiunea v2.0.4
* **Auto ROM Select inteligent:** Detectează automat dimensiunea fișierului. Dacă dimensiunea se potrivește cu o valoare standard, selectează presetarea corespunzătoare; altfel, transmite microcontrolerului adresa finală exactă pentru fișiere de dimensiuni arbitrare. Pentru capacități de peste 4 MB (8 MB și 16 MB), este necesară lipirea liniilor de adresă $A21/A22$.
* **Viteza de operare:** Citirea a 1 MB durează ~3.2 min. Din cauza limitărilor hardware ale Arduino Nano (16 MHz) și ale protocoalelor UART/I2C/SPI, scrierea este mai lentă — ~30 min pentru 1 MB.
* **Integrare hardware:** Magistrală de date de 16 biți prin MCP23017 (I2C) / MCP23S17 (SPI), magistrală de adrese prin cascadă 74HC595.
* **Byte Swapping bidirecțional:** Programatorul inversează automat bayții atât la citire, cât și la scriere. Fișierele `.bin`, `.gen`, `.md` sunt gata de utilizare imediat, fără utilitare externe.
* **Testare rapidă:** Modul `TEST 32KB` permite verificarea rapidă a magistralei și a cipului fără întârzieri.
* **Verificarea dump-ului:** Verificare opțională a datelor prin butonul `Verify` după scriere.
* **Protecția proceselor:** Blocarea modului de suspensie al sistemului de operare în timpul operațiunilor Flash.

---

</details>

---

## 🖥 Interfață și caracteristici software (Qt GUI)

![Interface](Interface.png)

* **Monitorizare:** Cronometru LCD pentru operațiuni și fereastră de jurnal pentru consolă.
* **Cardul Chip Information:** Panou informativ pe ecranul principal care afișează numele cipului detectat, ID-ul și memoria reală.
* **Selectarea capacității / Auto ROM Select:** Presetări de la 512 KB până la 32 MB, `TEST 32KB` (verificare rapidă) și modul inteligent **`Auto ROM Select`**.
* **Vizualizator Hex:** Vizualizare încorporată a dump-urilor de memorie pe 16 coloane (`Show buffer`).
* **Operațiuni principale:** Citire, Scriere, Ștergere, Verificare și Detectare automată a cipului.

---

## 🌟 Caracteristici cheie

* **Selectare automată a cipului și Auto-ID:** Detectarea automată a modelului și parametrilor memoriei prin identificatorii săi unici.
* **Detectarea automată a capacității după ID:** Programul corelează valorile MID/CID citite cu baza de date `chips.txt` și setează dimensiunea corectă.
* **Mapper SSF2 (până la 32 MB):** Suport pentru `Bank Switching` destinată imaginilor ROM mari. Necesită schițe speciale RP2040 cu sufixul **`"_SSFmap"`** (destinate **exclusiv** cartușelor cu mapper SSF2 încorporat). Pentru utilizarea schițelor standard fără mapper la capacități de peste 4 MB (8 MB și 16 MB), este obligatorie lipirea suplimentară a liniilor de adresă **`A21/A22`**.
* **Byte Swapping bidirecțional:** Programatorul inversează automat bayții în timpul citirii și scrierii.
* **Integrare hardware:** Magistrală de date de 16 biți prin MCP23017 (I2C) / MCP23S17 (SPI), magistrală de adrese prin cascadă 74HC595.
* **Viteza de scriere (1 MB):** ~5 minute pe Raspberry Pi Pico (față de ~30 minute pe Arduino Nano).
* **Protecția proceselor:** Blocarea modului de suspensie al sistemului de operare în timpul operațiunilor Flash și protecția bufferului împotriva depășirii capacității cipului.

---

## 🔌 Schema de conectare

### 1. Magistrala de adrese (74HC595)
| Semnal | RP2040 (Pico) | Arduino Nano |
| :--- | :--- | :--- |
| **SER (Data)** | GP2 | A0 |
| **L_CLK (Latch)** | GP3 | A1 |
| **S_CLK (Clock)** | GP6 | A2 |

### 2. Liniile de control Flash și Indicatori LED
| Semnal | Slot (Pin) | RP2040 (Pico) | Arduino Nano | Descriere |
| :--- | :--- | :--- | :--- | :--- |
| **LED (W/R)** | — | GP20 | D7 | Indicator LED pentru citire/scriere |
| **#WE** | B30 | GP7 | A3 | Write Enable (LOW) |
| **#CE** | B17 | GP8 | D8 | Chip Enable (LOW) |
| **#OE** | B16 | GP9 | D9 | Output Enable (LOW) |
| **#TIME** | B31 | GP21 | D5 | Time/Peripheral chip select |
| **#LWR** | B28 | GP22 | D6 | Lower Byte Write |
| **#RST** | B27 | RUN | RESET | Linie de resetare (Reset) |

### 3. Magistrala de date (MCP23S17 / MCP23017)

#### Modul MCP23017 (I2C)
| Semnal MCP23017 | RP2040 (Pico) | Arduino Nano | Descriere |
| :--- | :--- | :--- | :--- |
| **SDA** | GP4 | A4 | I2C Data Line |
| **SCL** | GP5 | A5 | I2C Clock Line |

#### Modul MCP23S17 (SPI — Recomandat)
| Semnal MCP23S17 | RP2040 (Pico) | Arduino Nano | Descriere |
| :--- | :--- | :--- | :--- |
| **CS** | GP17 | D10 | Chip Select |
| **SCK** | GP18 | D13 | SPI Clock |
| **MOSI (SI)** | GP19 | D11 | Master Out Slave In |
| **MISO (SO)** | GP16 | D12 | Master In Slave Out |

---

## 💾 Cipuri suportate

Programul acceptă o gamă largă de memorii Flash paralele pe 16 biți, inclusiv:
* **Macronix:** MX29LV400, MX29LV800, MX29LV160, MX29LV320, MX29LV640
* **AMD / Fujitsu / Spansion:** AM29LV800, MBM29LV400/800
* **STMicroelectronics / Eon / EliteMT:** M29W160, EN29LV160/800, ES29LV800, HY29F800 etc.
* **Spansion / Cypress:** Familia S29GL (S29GL256S, S29GL128S, S29GL064x, S29GL032x etc.)

*(Lista completă poate fi editată oricând în fișierul `chips.txt` aflat lângă program).*

---

## 🛠 Hardware

În repository veți găsi fișierele necesare pentru asamblarea dispozitivului (schema și PCB-ul au rămas stabile începând cu versiunea v3.0.0):
* **[Schema programatorului și Gerber:](Hardware/v3.0.0)** — Schema electrică principială și fișierele Gerber pregătite pentru fabricarea plăcii de circuit imprimat (PCB).

---

## 📦 Lansări și Software

* **Software pentru Windows:** În secțiunea **[Releases](../../releases/latest)** puteți descărca programul de control arhivat ca `16BitFlash_Programmer_v4.3.0.7z`, împreună cu codul sursă.
* **Prezentare video:** Puteți urmări procesul de lucru și exemple de utilizare pe [Canalul YouTube (Demonstrație video)](https://youtu.be/o0FTJ8lp1q4).

---

## 👨‍💻 Autori

* **Dezvoltator:** Pushkash
* **Asistent AI:** Gemini
