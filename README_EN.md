# 16BitFlash Programmer (SEGA Mega Drive / Genesis)

🌐 **Language:** [Русский](README.md) | [Română](README_RO.md) | English

![Version](https://img.shields.io/badge/version-v4.3.0-blue)
![Platform](https://img.shields.io/badge/platform-RP2040%20%7C%20Arduino%20Nano%20%7C%20Qt%206-green)
![License](https://img.shields.io/badge/license-MIT-orange)

> 💡 **Project History:** The project originally started as a simple experiment to study COM port communication, the UART protocol, and data transfer between a PC and external devices, but it has grown into a full-featured professional programmer.

A programmer powered by a **Raspberry Pi Pico (RP2040) or Arduino Nano** microcontroller and **Qt 6 C++** desktop software for **Reading, Writing, Erasing, Verifying, and Auto-ID** of 16-bit parallel Flash memory chips (**MX29LV, AM29LV, S29GL** series, etc.) and **SEGA Mega Drive / Genesis** cartridges.

---

## 🚀 What's New in v4.3.0

Version **v4.3.0** incorporates all thoroughly tested features from previous releases and introduces major updates for hardware compatibility and configuration flexibility:

### 1. «Chip ID» Button (Automatic Chip Detection)
* Implemented automatic detection of memory chips based on unique identifiers (MID and CID) read through the RP2040 or Arduino microcontroller.
* Information labels (`Chip Information`) added to the main UI, displaying the detected chip name, its ID, and actual memory capacity.
* When opening a larger file, program selection is allowed, but the buffer and settings are automatically truncated to the physical size of the chip.

### 2. External Chip Database (`chips.txt`)
* Added support for an external configuration file **`chips.txt`**, located alongside the program executable. The application automatically reads chip names, manufacturers (`MID`), device IDs (`CID`), and memory sizes from it.
* **Auto-creation of template:** If the file is missing, the program automatically generates it with a starter set of chips and comments. You can now easily add new chips in a plain text file without recompiling the source code.

### 3. Hardware Erase Monitoring via `DQ7` Bit
* Removed old fixed delays: the erase process is now monitored via the **`DQ7`** status bit with visual LED blinking on the RP2040 or Arduino board.
* Implemented reliable asynchronous readiness polling via signals with a guard timer on the PC side, keeping the user interface fully responsive during the erase cycle.

---

## 📜 Version History Archive (Changelog History)

<details>
<summary><b>🔽 Click to expand previous version history</b></summary>

### Version v3.1.1
* **Qt 6 GUI:** Added the ability to select and work with ROM images up to 32 MB directly in the program interface.
* **RP2040 Support:** New sketches added for Raspberry Pi Pico (RP2040) supporting Sega Bank Switching (SSF2 Mapper).
* **Reading & Writing (SSF2):** Stable support implemented for reading and writing up to 32 MB ROM images via the SSF2 mapper on hardware without soldering modifications (requires a cartridge with a built-in SSF2 mapper).

### Version v3.1.0
* **Software UI Update (Qt GUI):** Improved user interface, optimized ROM size management via a dropdown list (`QComboBox`).
* **Communication Protocol Optimization:** Desktop application adapted and optimized for the high data transfer speeds of the **RP2040**.

### Version v3.0.0
* **Transition to RP2040 (Raspberry Pi Pico):** 32-bit ARM Cortex-M0+ microcontroller.
* **PC Software Update (Qt GUI):** Full support and protocol optimization for the **RP2040** microcontroller added to the desktop app.
* **Speed Boost:** Writing time for **1 MB** dropped from **~30 minutes** (on Arduino Nano) to **~5 minutes** (on RP2040). Writing massive **15 MB** dumps takes only **~1 hour 10 minutes** instead of **~8 hours 20 minutes** (on Arduino Nano).
* **Updated Printed Circuit Board (PCB v3.0.0):** Dual-layer PCB with dual footprints (Pico + Nano), operation status LEDs, and support for fast **MCP23S17 (SPI)** / **MCP23017 (I2C)** expanders.

### Version v2.0.4
* **Smart Auto ROM Select:** Automatically detects file size. If the size matches a standard preset, it selects it; otherwise, it passes the exact end address to the microcontroller for arbitrary file sizes. Capacities above 4 MB (8 MB and 16 MB) require soldering address lines $A21/A22$.
* **Operation Speed:** Reading 1 MB takes ~3.2 min. Due to hardware limitations of the Arduino Nano (16 MHz), UART, and I2C/SPI protocols, writing is slower — ~30 min per 1 MB.
* **Hardware Integration:** 16-bit data bus via MCP23017 (I2C) / MCP23S17 (SPI), address bus via 74HC595 shift register cascade.
* **Bidirectional Byte Swapping:** The programmer automatically swaps bytes during both reading and writing. `.bin`, `.gen`, `.md` files are ready to use immediately without external utilities.
* **Fast Testing:** `TEST 32KB` mode allows quick bus and chip verification without delays.
* **Dump Verification:** Optional verification via the `Verify` button after writing.
* **Process Protection:** OS sleep mode blocking during Flash operations.

---

</details>

---

## 🖥 Interface and Software Features (Qt GUI)

![Interface](Interface.png)

* **Monitoring:** LCD operation timer and console log window.
* **Chip Information Card:** Information panel on the main screen showing the detected chip name, ID, and real memory size.
* **Capacity Selection / Auto ROM Select:** Presets from 512 KB up to 32 MB, `TEST 32KB` (quick test), and the smart **`Auto ROM Select`** mode.
* **Hex Viewer:** Built-in 16-column memory dump visualization (`Show buffer`).
* **Main Operations:** Read, Write, Erase, Verify, and Auto-ID.

---

## 🌟 Key Features

* **Auto Chip Selection & Auto-ID:** Automatic detection of memory model and parameters via unique chip identifiers.
* **Automatic Capacity Detection by ID:** The program maps read MID/CID values against `chips.txt` to set the correct size.
* **SSF2 Mapper (up to 32 MB):** `Bank Switching` support for large ROM images. Requires special RP2040 sketches with the **`"_SSFmap"`** suffix (intended **strictly** for cartridges with a built-in SSF2 mapper). Using standard sketches without a mapper for sizes above 4 MB (8 MB and 16 MB) requires mandatory soldering of additional address lines **`A21/A22`**.
* **Bidirectional Byte Swapping:** Automatic byte reordering during read and write operations.
* **Hardware Integration:** 16-bit data bus via MCP23017 (I2C) / MCP23S17 (SPI), address bus via 74HC595 cascade.
* **Write Speed (1 MB):** ~5 minutes on Raspberry Pi Pico (vs. ~30 minutes on Arduino Nano).
* **Process Protection:** OS sleep mode blocking during Flash operations and buffer protection against exceeding chip capacity.

---

## 🔌 Connection Schema

### 1. Address Bus (74HC595)
| Signal | RP2040 (Pico) | Arduino Nano |
| :--- | :--- | :--- |
| **SER (Data)** | GP2 | A0 |
| **L_CLK (Latch)** | GP3 | A1 |
| **S_CLK (Clock)** | GP6 | A2 |

### 2. Flash Control Lines and Status LED
| Signal | Slot (Pin) | RP2040 (Pico) | Arduino Nano | Description |
| :--- | :--- | :--- | :--- | :--- |
| **LED (W/R)** | — | GP20 | D7 | Read/Write process LED indicator |
| **#WE** | B30 | GP7 | A3 | Write Enable (LOW) |
| **#CE** | B17 | GP8 | D8 | Chip Enable (LOW) |
| **#OE** | B16 | GP9 | D9 | Output Enable (LOW) |
| **#TIME** | B31 | GP21 | D5 | Time/Peripheral chip select |
| **#LWR** | B28 | GP22 | D6 | Lower Byte Write |
| **#RST** | B27 | RUN | RESET | Reset line |

### 3. Data Bus (MCP23S17 / MCP23017)

#### MCP23017 (I2C Mode)
| MCP23017 Signal | RP2040 (Pico) | Arduino Nano | Description |
| :--- | :--- | :--- | :--- |
| **SDA** | GP4 | A4 | I2C Data Line |
| **SCL** | GP5 | A5 | I2C Clock Line |

#### MCP23S17 (SPI Mode — Recommended)
| MCP23S17 Signal | RP2040 (Pico) | Arduino Nano | Description |
| :--- | :--- | :--- | :--- |
| **CS** | GP17 | D10 | Chip Select |
| **SCK** | GP18 | D13 | SPI Clock |
| **MOSI (SI)** | GP19 | D11 | Master Out Slave In |
| **MISO (SO)** | GP16 | D12 | Master In Slave Out |

---

## 💾 Supported Chips

The program supports a wide range of 16-bit parallel Flash memories, including:
* **Macronix:** MX29LV400, MX29LV800, MX29LV160, MX29LV320, MX29LV640
* **AMD / Fujitsu / Spansion:** AM29LV800, MBM29LV400/800
* **STMicroelectronics / Eon / EliteMT:** M29W160, EN29LV160/800, ES29LV800, HY29F800, etc.
* **Spansion / Cypress:** S29GL family (S29GL256S, S29GL128S, S29GL064x, S29GL032x, etc.)

*(The complete list can always be edited in the `chips.txt` file located next to the program).*

---

## 🛠 Hardware

The repository contains files for self-assembly of the device (schematic and PCB layouts remain stable since v3.0.0):
* **[Programmer Schematic and Gerber:](Hardware/v3.0.0)** — Circuit schematic and ready-to-use Gerber files for ordering PCB manufacturing.

---

## 📦 Releases and Software

* **Windows Software:** The control software archive `16BitFlash_Programmer_v4.3.0.7z` and source code are available in the **[Releases](../../releases/latest)** section.
* **Video Overview & Demo:** Watch the operation process and usage examples on the [YouTube Channel (Video Demonstration)](https://youtu.be/o0FTJ8lp1q4).

---

## 👨‍💻 Authors

* **Developer:** Pushkash
* **AI Assistant:** Gemini
