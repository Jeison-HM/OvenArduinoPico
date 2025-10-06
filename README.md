<div align="center">
<h1>Oven Controller on Arduino</h1>
</div>

<!-- GitHub Badges Section -->
<p align="center">
  <img alt="C++" height="24.5px" style="padding-right:5px;" src="https://custom-icon-badges.demolab.com/badge/-C++-005697?style=for-the-badge&logo=cpp&logoColor=white&logoSize=auto"/>
  <img alt="Python" height="24.5px" style="padding-right:5px;" src="https://custom-icon-badges.demolab.com/badge/python-3670A0?style=for-the-badge&logo=python&logoColor=ffdd54"/>
  <img alt="Arduino" height="24.5px" style="padding-right:5px;" src="https://custom-icon-badges.demolab.com/badge/-Arduino-16969b?style=for-the-badge&logo=arduino&logoColor=white&logoSize=auto"/>
  <img alt="Pico" height="24.5px" style="padding-right:5px;" src="https://custom-icon-badges.demolab.com/badge/-Pico-bd1b48?style=for-the-badge&logo=raspberrypi&logoColor=white&logoSize=auto"/>
  <img alt="MIT" height="24.5px" style="padding-right:5px;" src="https://custom-icon-badges.demolab.com/badge/License-MIT-ad0808?style=for-the-badge"/>
    <br />
</p>

<!-- Description -->
<p align="center">
    <b>Oven Controller using Arduino UNO and Raspberry Pi Pico Integration</b><br>
    <i>with <code>Bluetooth</code> CLI.</i>
</p>

## 📘 Overview

This project implements an **oven control system** using an **Arduino Uno** and **LM335 temperature sensor**, featuring a **Bluetooth (BT) command-line interface (CLI)** for real-time control via smartphone and a **Raspberry Pi Pico integration** for performing remote arithmetic calculations.

The oven automatically controls the heater based on a **set temperature**, provides **state indication LEDs**, and includes **error handling, safety measures, and command validation**.

### 🧾 Example Session

```
> Proyecto Horno por grupo 4
> TEMP=30
Temperature set to = 30 C
> TIME=10
Time set to = 10 s
> START
Start : ON
Temperature = 28 C
Temperature = 29 C
Temperature = 30 C
Time = 10 s
Temperature = 30 C
Time = 9 s
...
Process finish!
```

## ⚙️ System Features

### 🔧 Main Components

<div align="center">

| Component                            | Function                                                        |
| ------------------------------------ | --------------------------------------------------------------- |
| **Arduino Uno**                      | Main controller managing temperature, heater, and communication |
| **LM335**                            | Analog temperature sensor (ADC-based)                           |
| **Bluetooth Module (HC-05 / HC-06)** | Wireless CLI communication with smartphone                      |
| **LED Indicators**                   | Power, Stop/Error, and Temperature Status indicators            |
| **Heater (or motor/LED)**            | Simulated oven heater element                                   |
| **Buzzer**                           | Audible feedback for process end and errors                     |
| **Raspberry Pi Pico**                | Performs arithmetic calculations and communicates via UART      |

</div>

## 🧠 System Behavior

The system operates through **three main states** managed by a mealy **finite state machine (FSM):**

### 🕹️ 1. IDLE

* Default standby state.
* Awaits configuration and commands from the user via Bluetooth.
* Accepts commands.

### 🔥 2. PRE-HEAT

* The oven starts reading the **LM335 temperature** through ADC sampling (`A5` pin).
* Compares the current temperature to the set point with a **±5°C margin**.
* LED indicators:

  * **LOW LED** → below target range
  * **SET LED** → within target range
  * **HIGH LED** → above target range
* Transitions to `HEATING` once within the target range.

### ⏱️ 3. HEATING

* The heater remains ON while counting down the configured time.
* Displays remaining time and temperature every second.
* On completion:

  * Heater turns OFF
  * Buzzer emits two short beeps
* If temperature exceeds range:

  * System triggers **emergency STOP**
  * Buzzer emits three long beeps
  * User must reset or send `STOP` command to resume.

## 🚨 Safety & Error Handling

* **STOP Command** acts as a toggle (latched behavior).

  * Can be triggered at **any time or state**.
  * Errors automatically trigger a STOP (sets `stop = true`).
* **Invalid Commands** or malformed inputs are handled gracefully with error messages.
* **Error conditions** (like temperature out of range) immediately stop the heater and notify via Bluetooth.

## 📱 Bluetooth CLI Interface

The Bluetooth module (HC-05/HC-06) is connected via **SoftwareSerial** (`RX: 2`, `TX: 3`).
Baud rate: **115200**

### 💬 Supported Commands

<div align="center">

| Command                 | Description                                    | Example           |
| ----------------------- | ---------------------------------------------- | ----------------- |
| `TEMP=XX`               | Set desired temperature (°C)                   | `TEMP=50`         |
| `TIME=XX`               | Set heating time (s)                           | `TIME=30`         |
| `START`                 | Starts heating (only if TEMP and TIME are set) | `START`           |
| `STOP`                  | Emergency stop (toggles ON/OFF)                | `STOP`            |
| `SHOW`                  | Displays system status                         | `SHOW`            |
| `HELP`                  | Lists all commands                             | `HELP`            |
| `Calcular: Num1 S Num2` | Perform calculation via Raspberry Pi Pico      | `Calcular: 8 + 5` |

</div>

## 🔢 Raspberry Pi Pico Integration

A **bonus feature** of this project integrates a **Raspberry Pi Pico** via UART (`RX: 4`, `TX: 5`) to perform **simple arithmetic calculations**.

### 📡 Communication Flow

1. User sends:
   `Calcular: Num1 S Num2` (e.g. `Calcular: 8 * 4`)
2. Arduino transmits the expression to the Pico.
3. Pico computes the result and sends back:
   `8 * 4 = 32`
4. Arduino relays the response to the Bluetooth terminal and prints it in the Python console.

### 💻 Pico Code Summary

* UART @ **115200 baud**, 8N2 configuration.
* Parses received string to identify operator (`+`, `-`, `*`, `/`).
* Computes result and sends back formatted string.
* Handles malformed inputs or division by zero errors.

## 🔋 System Timing

**TimerOne** is configured to trigger every **250 ms** for:

  * Power LED blinking
  * Input reading via Bluetooth

## 🔔 Buzzer Feedback

<div align="center">

| Event                      | Pattern          |
| -------------------------- | ---------------- |
| **Process Completed**      | Two short beeps  |
| **Error / Emergency STOP** | Three long beeps |

</div>