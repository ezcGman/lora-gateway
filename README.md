# LoRa-To-MQTT Gateway
## Introduction
This repository gives your everything you need to easyily build (It's a DIY project!) a great quality LoRa-To-MQTT gateway, based on EByte LoRA modules and an ESP32 and works either with Wi-Fi or Ethernet, running off 5V. There are two different versions of the gateway, details on those versions and which you should pick down below:
- One with an EByte E32
- One with EByte E220

For giving the board Ethernet capability, I'm using the [QuinLED ESP32 Ethernet Hat](https://quinled.info/quinled-esp32-ethernet/). Of course, It's natural that I *should* have also used a [QuinLed ESP32](https://quinled.info/quinled-esp32/), but I had a few spare standard ESP32 devboards laying around with no use, so I designed the board around these. But that said, I might one day design a version completely based on the QuinLED-ESP32.

To easily switch between Wi-FI and Ethernet, there's a jumper on the board to do that. This even works with the board powered; no need to cut power.

## What do I need to do to get one?
Again, this is a DIY project! So first off, you need a few soldering skills and I highly recommend either a SMD hot plate or hot air gun to solder the Ebyte E32 module. Its **impossible** to solder it with just an iron.

### 1. Get the PCBs
So you want to get the PCBs printed at a PCB prototype factory of your choice, like JLCPCB or PCBWay. I've included the Gerber files for both in the respectiv folder. If you want to use a different service provider, you need to check if they may accept these gerbers or generate them yourself.

### 2. Get the components
**TBD:** I'll generate a BOM file per board version and throw it into its respective folder

### 3. Build the board and wire it up
Each PCB folder has an iBOM HTML file which gives you nice soldering instructions / overview, find it in the `ibom` folder in each versions folder. When you've soldered the board, simply wire it up with a 5V power source and there you go! It consumes less than 200mA, so you can easily use an old 500mA or 1A phone charger for it.

### 4. Install the software
Source is available in the `src` folder. Download Arduino IDE, check the `config.h` file and replace the placeholders with your settings, compile and upload to the ESP32. That should be it and the gateway should pop up in your MQTT server and send health check messages every 5 seconds.

How you can now have your sensors and boards have LoRa messages sent to it can be found in the "Source Code / Software" section below.

## PCBs
### General design
- Can either be operated using Wi-Fi or Ethernet
    - Jumper on board to (even with powered-on board) switch between those two
- Uses a standard **30-pin (2x15-pin)** ESP32 DevKit
- For Ethernet, uses the [QuinLED ESP32 Ethernet Hat](https://quinled.info/quinled-esp32-ethernet/)
- Easily powered using a screw terminal
- Extra protection using a PTC fuse, in case something goes sideways
- *Can* also be powered via USB on the ESP32, but this will skip the PTC fuse and with that your protection. So **not recommended**
- Four M2.5 mounting holes to mount it to or into whatever you want

Which one should you pick? That's pretty easy and defined by one factor:
- If you / your sensors and other boards use the RFM95 modules and the Arduino-LoRa library: Use the E32 version!
- If you usually use EByte modules that use serial communication and Renzo Mischiantis [famous Arduino library](https://github.com/xreef/LoRa_E32_Series_Library): Use the E220 version.

You need to choose between those, as the EByte modules that use serial communication can't communicate with the RFM95 modules, although they use the same Semtech SX1276 LoRa chip. That's because the EByte serial modules also have a litte MCU onboard that already does its own "stuff" / protocol when sending messages via LoRa. One would need to understand how exactly that works to read these messages with a LoRa modules that uses direct SPI communication to the Semtech chip (like the specific E32 module I use, or the RFM95) to make them work with each other.

### LoRa-Gateway-E32
![LoRa-Gateway-E220](/pcbs/LoRa-Gateway-E220/LoRa-Gateway-E220.png)
This version is based on the EByte E32-400M20S or E32-900M20S, which offers direct serial communicaton to the Semtech SX1276. You need to use this specific module not any other that uses serial communication. Pick the right one for your region (tl;dr: 400 for US/Asia, 900 for Europe).

The module is **not** in the BOM CSV file, you need to get it off AliExpress:
- 400MHz:
    - EBYTE IOT Factory Store: https://www.aliexpress.com/item/1005003567891629.html
    - cojxu Official Store: https://www.aliexpress.com/item/1005003567929089.html
- 900MHz
    - EBYTE IOT Factory Store: https://www.aliexpress.com/item/1005003509312425.html
    - cojxu Official Store: https://www.aliexpress.com/item/1005003646639064.html

The software for this version is available in its first alpha v0 in the `src` folder.

### LoRa-Gateway-E220
![LoRa-Gateway-E220](/pcbs/LoRa-Gateway-E32/LoRa-Gateway-E32.png)
If you usually work in the EByte ecosystem with its serial interfaced modules, this board is for you. **However** Iv'e not done a port of my E32 based software for this board. And I'm actually not planning to do so, as I personally use the E32 version. But you're welcome to port it using Renzo Mischiantis [Arduino library](https://github.com/xreef/LoRa_E32_Series_Library).

The module is **not** in the BOM CSV file, you need to get it off AliExpress:
- 400MHz:
    - EBYTE IOT Factory Store: https://www.aliexpress.com/item/1005003567891629.html
    - cojxu Official Store: https://www.aliexpress.com/item/1005003567929089.html
- 900MHz
    - EBYTE IOT Factory Store: https://www.aliexpress.com/item/1005003509312425.html
    - cojxu Official Store: https://www.aliexpress.com/item/1005003646639064.html

## Source code
Docs TBD
