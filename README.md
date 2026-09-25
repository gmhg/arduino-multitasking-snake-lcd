# arduino-multitasking-snake-lcd
# Arduino Multi-tasking Snake Game (Samsung IR Remote Control)

An ultra-optimized, non-blocking implementation of the classic **Snake game** tailored for an **Arduino Uno** using a standard **16x2 LCD display** and controlled via a standard **Samsung TV IR Remote**.

This project was originally developed 7 years ago and has been brought back to GitHub to serve as an example of efficient embedded systems design and multi-tasking without an RTOS.

## 🛠️ The Engineering Behind It

Unlike common beginner projects, this code achieves high efficiency and unique features by implementing deep low-level optimizations:

* **Pseudo 16x4 Resolution on a 16x2 Display:** By custom-defining the HD44780 LCD's 5x8 pixel matrices, each standard cell is split into two independent 4x4 sub-characters. This dynamically expands the playable area to a 16x4 grid. Special hybrid custom characters handle rendering when food and the snake share the same character cell.
* **Non-Blocking Cooperative Multitasking:** Strictly avoids the use of `delay()`. The game loop runs asynchronously, relying on `millis()` intervals to advance the snake's position while leaving processing cycles free for real-time IR reading.
* **Custom FSM for Samsung IR Protocol:** Features a built-in Finite State Machine (FSM) that decodes 32-bit Samsung infrared signals on-the-fly using edge detection (`micros()`). It filters start bits and pulse distances without blocking the CPU execution window.
* **Circular Buffer Data Structures:** Snake body parts are stored in 64-element arrays utilizing efficient bitwise masking (`& 63`) to handle directional updates, saving CPU cycles during motion shifts.

## 🔌 Hardware Setup

* **Microcontroller:** Arduino Uno (ATmega328P) or equivalent.
* **Display:** 16x2 Character LCD (Parallel interface via `LiquidCrystal` library).
* **Sensor:** Infrared Receiver (e.g., TSOP38238) connected to Pin `A5`.
* **Input:** Any standard Samsung TV Remote Control.

### LCD Pin Mapping
`LiquidCrystal lcd(12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2);`

### IR Codes mapped:
* **UP:** `0x9F`
* **DOWN:** `0x9E`
* **LEFT:** `0x9A`
* **RIGHT:** `0x9D`
* **ENTER (Restart):** `0x97`

## 🚀 How to Run

1. Clone or download this repository.
2. Open the file in the **Arduino IDE**.
3. Wire the circuit according to the pin configurations in the source code.
4. Upload the code to your Arduino.
5. Point your Samsung TV remote at the IR sensor and play!
