
# IRMP - Infrared Multi Protocol Decoder
### Version 1.2.2
[![License: GPL v3](https://img.shields.io/badge/License-GPLv3-blue.svg)](https://www.gnu.org/licenses/gpl-3.0)
[![Build Status](https://github.com/ukw100/irmp/workflows/build/badge.svg)](https://github.com/ukw100/irmp/actions)
[![Hit Counter](https://hitcounter.pythonanywhere.com/count/tag.svg?url=https%3A%2F%2Fgithub.com%2Fukw100%2FIRMP)](https://github.com/brentvollebregt/hit-counter)

## 50 IR protocols supported and low memory footprint
40 protocols can be enabled at the same time, since some of them are quite similar and conflicts with each other

| Nano running AllProtocol example | YouTube Video | Instructable |
|-|-|-|
| ![Nano running AllProtocol example](https://github.com/ukw100/IRMP/blob/master/pictures/NEC.jpg) | ![YouTube Video](https://github.com/ukw100/IRMP/blob/master/pictures/KASEIKYO+Remote.jpg) | [![Instructable](https://github.com/ArminJo/Arduino-OpenWindowAlarm/blob/master/pictures/instructables-logo-v2.png)](https://www.instructables.com/id/IR-Remote-Analyzer-Receiver-With-Arduino) |

# Documentation at mikrocontroller.net
### English
   http://www.mikrocontroller.net/articles/IRMP_-_english
### German
   http://www.mikrocontroller.net/articles/IRMP
   
# List of protocols
- Sony SIRCS, NEC + APPLE + ONKYO, Samsung + Samsg32, Kaseikyo
- JVC, NEC16, NEC42, Matsushita, DENON, Sharp, RC5, RC6 & RC6A, IR60 (SDA2008) Grundig, Siemens Gigaset, Nokia
- BOSE, Kathrein , NUBERT , FAN (ventilator) , SPEAKER (~NUBERT), Bang & Olufsen , RECS80 (SAA3004) , RECS80EXT (SAA3008), Thomson, NIKON camera , Netbox keyboard, ORTEK (Hama) , Telefunken 1560, FDC3402 keyboard , RC Car , iRobot Roomba, RUWIDO, T-Home , A1 TV BOX, LEGO Power RC, RCMM 12,24, or 32, LG Air Condition , Samsung48, Merlin , Pentax , S100 , ACP24, TECHNICS , PANASONIC Beamer , Mitsubishi Aircond , VINCENT, SAMSUNG AH , IRMP specific, GREE CLIMATE , RCII T+A, RADIO e.g. TEVION, METZ<br/>
- NEC, Kaseiko, Denon, RC6, Samsung + Samsg32 were sucessfully tested in interrupt mode.

# Timer usage
The IRMP library works by polling the input pin at a rate of 10000 to 20000 Hz. Default is 15000 Hz.<br/>
**Some protocols can be detected by just using interrupts from the input pin** instead of polling. In this case - `IRMP_ENABLE_PIN_CHANGE_INTERRUPT` is defined - **no timer is needed**. See [Interrupt example](https://github.com/ukw100/IRMP/blob/master/examples/Interrupt/Interrupt.ino).<br/>
- For AVR timer 2 (Tone timer) is used. For variants, which have no timer 2 like ATtiny85 or ATtiny167, timer 1 is used.
- For ESP8266 and ESP32 timer1 is used.
- For STM32 (BluePill) timer 3 (Servo timer) channel 1 is used as default.<br/>
- The `millis()` function and the corresponding timer is not used by IRMP!

# Schematic for Arduino UNO
| IR-Receiver connection | Serial LCD connection |
|---|---|
![Fritzing schematic for Arduino UNO](https://github.com/ukw100/IRMP/blob/master/extras/IRMP_UNO_Steckplatine.png) | ![Fritzing schematic for Arduino UNO + LCD](https://github.com/ukw100/IRMP/blob/master/extras/IRMP_UNO_LCD_Steckplatine.png)

# Quick comparison of 4 Arduino IR receiving libraries
## This is a short comparison and may not be complete or correct
I created this comparison matrix for [myself](ArminJo) in order to choose a small IR lib for my project and to have a quick overview, when to choose which library.<br/>
It is dated from **27.2.2020**. If you have complains about the data or request for extensions, please send a PM or open an issue.

| Subject | [IRMP](https://github.com/ukw100/IRMP) | [IRLremote](https://github.com/NicoHood/IRLremote) | [IRLib2](https://github.com/cyborg5/IRLib2) | [IRremote](https://github.com/z3t0/Arduino-IRremote) |
|---------|------|-----------|--------|----------|
| Number of protocols | **52** | Nec + Panasonic + Hash \* | 12 + Hash \* | 13 |
| 3.Party libs needed| % | PinChangeInterrupt if not pin 2 or 3 | % | % |
| Timing method | Timer2 or interrupt for pin 2 or 3 | **Interrupt** | Timer2 or interrupt for pin 2 or 3 | Timer2 |
| Decode method | OnTheFly | OnTheFly | RAM | RAM |
| FLASH usage (simple NEC example with 5 prints) | 1500<br/>(4300 for 15 main / 8000 for all 40 protocols)<br/>(+200 for callback)<br/>
(+80 for interrupt at pin 2+3)| **1270**<br/>(1400 for pin 2+3) | 4830 | 3210 |
| RAM usage | **52**<br/>(73 / 100 for 15 (main) / 40 protocols) | **62** | 334 | 227 |
| Supported platforms | **avr, attiny, Digispark (Pro), esp8266, ESP32, STM32<br/>(plus arm and pic for non Arduino IDE)** | avr, esp8266 | avr, arm(samd) | avr, attiny, *esp8266*, esp32, arm(some boards) |
| Last library update | 2/2020 | 4/2018 | 9/2019 | 11/2017 |
| Remarks | LED 13 Feedback.<br/>Decodes 40 protocols concurrently.<br/>Work in progress. | Only one protocol at a time. | LED 13 Feedback. | LED 13 Feedback.<br/>NEC decoding is poor.<br/>**Abandoned project -209 open issues- and therefore an incredible amount of forks.** |

\*The Hash protocol gives you a hash as code, which may be sufficient to distinguish your keys on the remote, but may not work with some protocols like Mitsubishi

# Easy migrating your code from IRremote to IRMP
See also the [SimpleReceiver example](https://github.com/ukw100/IRMP/blob/master/examples/SimpleReceiver/SimpleReceiver.ino) .

### Change the include and declarations from:
```
#include <IRremote.h>
#define IR_RECEIVER_PIN 3
IRrecv myReceiver(IR_RECEIVER_PIN);
decode_results results;
```
to
```
#define IRMP_INPUT_PIN 3
#define IRMP_PROTOCOL_NAMES 1
#include <irmpSelectMain15Protocols.h>
#include <irmp.c.h>
IRMP_DATA irmp_data[1];
```

### Change in setup:
`myReceiver.enableIRIn();` to `irmp_init();`

### Changes to get the data:
`if (myReceiver.decode(&results))` to `if (irmp_get_data(&irmp_data[0]))`<br/>
and<br/>
`switch (results.value)` to `switch (irmp_data[0].command)`.

You do not need **`myReceiver.resume();`** any more, just delete it.

The IR code representation of IRMP is different from that in IRremote. In IRMP (as in IRLremote) it is more standard and simpler. Use the function `irmp_result_print(&irmp_data[0])` to print the IR code representation. Seee [SimpleReceiver example](https://github.com/ukw100/IRMP/blob/master/examples/SimpleReceiver/SimpleReceiver.ino).

If you want to distinguish between more than one remote in one sketch, you may also use `irmp_data[0].address` like it is done in the [Callback example](https://github.com/ukw100/IRMP/blob/master/examples/Callback/Callback.ino).

# AllProtocol example
| Serial LCD output | Arduino Serial Monitor output |
|-|-|
| ![LCD start](https://github.com/ukw100/IRMP/blob/master/pictures/Start.jpg) | ![Serial Monitor](https://github.com/ukw100/IRMP/blob/master/pictures/AllProtocol_SerialMonitor.png) |

## Sample Protocols
| | | | |
|-|-|-|-|
| ![NEC](https://github.com/ukw100/IRMP/blob/master/pictures/NEC_Paralell.jpg)| ![NEC42](https://github.com/ukw100/IRMP/blob/master/pictures/NEC42.jpg) |![RC5](https://github.com/ukw100/IRMP/blob/master/pictures/RC5.jpg) |![KASEIKYO](https://github.com/ukw100/IRMP/blob/master/pictures/KASEIKYO.jpg) |
| ![DENON](https://github.com/ukw100/IRMP/blob/master/pictures/DENON.jpg) |![GRUNDIG](https://github.com/ukw100/IRMP/blob/master/pictures/GRUNDIG.jpg) |![IR60](https://github.com/ukw100/IRMP/blob/master/pictures/IR60.jpg) |![MATSUSHITA](https://github.com/ukw100/IRMP/blob/master/pictures/MATSUSHITA.jpg) |
| ![NUBERT](https://github.com/ukw100/IRMP/blob/master/pictures/NUBERT.jpg) |![ONKYO](https://github.com/ukw100/IRMP/blob/master/pictures/ONKYO.jpg) |![RECS80](https://github.com/ukw100/IRMP/blob/master/pictures/RECS80.jpg) |![RUWIDO](https://github.com/ukw100/IRMP/blob/master/pictures/RUWIDO.jpg) |
| ![SAMSUNG](https://github.com/ukw100/IRMP/blob/master/pictures/SAMSUNG.jpg) |![SIEMENS](https://github.com/ukw100/IRMP/blob/master/pictures/SIEMENS.jpg) |![TELEFUNKEN](https://github.com/ukw100/IRMP/blob/master/pictures/TELEFUNKEN.jpg) |![TELEFUNKEN](https://github.com/ukw100/IRMP/blob/master/pictures/TELEFUNKEN.jpg) |


# Revision History
### Version 1.2.2
- Fixed bugs introduced in 1.2.1
 
### Version 1.2.1
- Bug for AVR architecture fixed.
- ATtiny85 + ATtiny167 support for ATTinyCore and Digistump core.
- Support for "Generic STM32F1 series" from STM32 Boards from STM32 cores of Arduino Board manager.

### Version 1.2.0 - This version contains a bug for the AVR architecture
- Added STM32 M3 (BluePill) support.

### Version 1.1.0
- Added functions `irmp_disable_timer_interrupt()` and `irmp_enable_timer_interrupt()`.
- Added function `irmp_result_print(Stream * aSerial)`.
- Improved examples.

### Version 1.0.1
- Added ESP8266 + ESP32 support.

# Travis CI
The IRMP library examples are built on Travis CI for the following boards:

- Arduino Uno
- ESP8266 boards (tested with LOLIN D1 R2 board)
- ESP32   boards (tested with ESP32 DEVKITV1 board)
- BluePill (STM32F103C)
- Digispark (ATtiny85)
- Digispark Pro (ATtiny167)
