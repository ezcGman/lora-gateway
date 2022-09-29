# LoRa-To-MQTT Gateway
## Introduction
This repository gives you everything you need to easily build (it's a DIY project!) a great quality LoRa-To-MQTT gateway, based on EByte LoRa modules and an ESP32 and works either with Wi-Fi or Ethernet, running off 5V. There are two different versions of the gateway, details on those versions and which you should pick down below:
- One using an EByte E32
- One using an EByte E220

For giving the board Ethernet capability, I'm using the [QuinLED ESP32 Ethernet Hat](https://quinled.info/quinled-esp32-ethernet/). Of course, It's natural that I *should* have also used a [QuinLed ESP32](https://quinled.info/quinled-esp32/), but I had a few spare standard ESP32 devboards laying around with no use for, so I designed the board around these. But that said, I might one day design a version completely based on the QuinLED-ESP32.

To easily switch between Wi-FI and Ethernet, there's a jumper on the board to do just that. This even works with the board powered; no need to cut power.

## What do I need to do to get one?
Again, this is a DIY project! So first off, you need a few soldering skills and I highly recommend either a SMD hot plate or hot air gun to solder the Ebyte E32 module. Its **impossible** to solder it with just an iron.

### 1. Get the PCBs
So you want to get the PCBs printed at a PCB prototype factory of your choice, like JLCPCB or PCBWay. I've included the Gerber files for both in the respectiv folder. If you want to use a different service provider, you need to check if they may accept these gerbers or generate them yourself.

I also **highly recommend** that you order this PCB with a stencil, otherwise you gonna have a hard time putting the paste on the pads of the EByte module!

### 2. Get the components
* Look up the iBOM files to get a list of components you need:
    * [E32 version iBOM](https://github.com/ezcGman/lora-gateway/blob/master/pcbs/LoRa-Gateway-E32/ibom/LoRa-Gateway-E32.html)
    * [E220 version iBOM](https://github.com/ezcGman/lora-gateway/blob/master/pcbs/LoRa-Gateway-E220/ibom/LoRa-Gateway-E220.html)
* Additional links for the ESP32 and QuinLED Ethernet hat:
    * Regular ESP 32 DevKit (pick one with 2x15 pins!!): https://www.aliexpress.com/wholesale?catId=0&initiative_id=SB_20220311122647&SearchText=esp32+devkit
    * *(optional, if you want Ethernet)* QuinLED ESP32 with Ethernet hat:
        * Worldwide store: https://shop.allnetchina.cn/collections/quinled/products/quinled-esp32
        * US only: https://drzzs.com/shop/quinled-esp32/

### 3. Build the board and wire it up
Each PCB folder has an iBOM HTML file which gives you nice soldering instructions / overview, find it in the `ibom` folder in each versions folder. When you've soldered the board, simply wire it up with a 5V power source and there you go! It consumes less than 200mA, so you can easily use an old 500mA, 1A (or higher) phone charger for it.

### 4. Install the software
Source is available in the `src` folder. Download Arduino IDE, check the `config.h` file and replace the placeholders with your settings, compile and upload to the ESP32. That should be it and the gateway should pop up in your MQTT server and send health check messages every 5 seconds.

How you can now have your sensors and boards have LoRa messages sent to it can be found in the ["Source Code / Software"](README.md#source-code--software) section below.

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

You need to choose between those, as the EByte modules that use serial communication can't communicate with the RFM95 modules, although they use the same Semtech SX1276 LoRa chip. That's because the EByte serial-operated modules also have a litte MCU onboard that already does its own "stuff" / protocol when sending messages via LoRa. So you don't talk directly to the Semtech chip, but that little MCU instead. One would need to understand how exactly that works to read these messages with a LoRa modules that uses direct SPI communication to the Semtech chip (like the specific E32 module I use, or the RFM95) to make them work with each other.

### LoRa-Gateway-E32
![LoRa-Gateway-E32](/pcbs/LoRa-Gateway-E32/LoRa-Gateway-E32.png)
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
![LoRa-Gateway-E220](/pcbs/LoRa-Gateway-E220/LoRa-Gateway-E220.png)
If you usually work in the EByte ecosystem with its serial interfaced modules, this board is for you. **However** I've not done a port of my E32 based software for this board. And I'm actually not planning to do so, as I personally use the E32 version. But you're welcome to port it using Renzo Mischiantis [Arduino library](https://github.com/xreef/LoRa_E32_Series_Library).

The module is **not** in the BOM CSV file, you need to get it off AliExpress:
- 400MHz:
    - EBYTE IOT Factory Store: https://www.aliexpress.com/item/1005002090403425.html
    - cojxu Official Store: https://www.aliexpress.com/item/1005002060190690.html
- 900MHz
    - EBYTE IOT Factory Store: https://www.aliexpress.com/item/1005002116885160.html
    - cojxu Official Store: https://www.aliexpress.com/item/1005003681666059.html

## Source Code / Software
The software I've developed for this is meant to be easily extensible without touching the actual logic of the gateway itself, but instead adding a little piece of code to a separate file.

Take a look at [`lora-ids.h`](/src/lora-gateway-e32/lora-ids.h). This file is meant to be shared between this gateway and all sensors you develop. It has a list of different message types and you can easily add your own. E.g., it has a "mailbox" type which defines a few properties a mailbox would send. This makes it very easy to read and process this messages **and** create topics in your MQTT server which you then can listen to. There's also a "custom" type which basically has free text and is maybe good for debugging. 

Feel free to add your own types in this file. You will need to add two things:
* An ID for it, like `#define LORA_MESSAGE_ID_CUSTOM 0x0`
* Define the struct that describes its properties: `struct LoRaMessageCustom : LoRaBase`

### How do the MQTT topics look like I can subscribe to?
The subject where a message falls into is built out of the gateway ID, device ID and message ID. So for the following example we assume that:
* Device with ID `0xA` and name `mailbox-sensor` has sent
* a message of type `LORA_MESSAGE_ID_MAILBOX` and
* the gateway with ID `0x1` and name `lora-gateway-e32` receives it.

So let's first take the `mailbox` message type as an example. It define these properties:
* long duration
* float distance
* float humidity
* float temperature

Additionally, it defines its own name (and with that: The parent for these properties):
* `String getMqttTopicName() { return "mailbox"; }`

So whenever the gateway will receive a message of this type, it will read the properties from it and drop them into these topics:
* `???/messages/mailbox/duration`
* `???/messages/mailbox/distance`
* `???/messages/mailbox/humidity`
* `???/messages/mailbox/temperature`

But where does the `/messages/` part come form and what is `???/`. That's easy: Easy device / sensor you develop should also be registered in `lora-ids.h`. At the very top, there is list of devices, each with:
* Device ID
* Device name (which affectively is used to construct the MQTT topic)

The gateway will sort all messages it receives by device and prefixes it with that device name, *plus* sorts all messages under a `messages` topic to allow the device have other topics next to the actual messages. This allows you to listen to specific fields/values of specific message types from a specific device!

So we now know this:
* `???/mailbox-sensor/messages/mailbox/duration`
* `???/mailbox-sensor/messages/mailbox/distance`
* `???/mailbox-sensor/messages/mailbox/humidity`
* `???/mailbox-sensor/messages/mailbox/temperature`

The last missing piece is the gateway that receives the messages. It's also defined as a device, so it also has a name, so we can easily construct the full topic name for our example:
* `lora-gateway-e32/devices/mailbox-sensor/messages/mailbox/duration`
* `lora-gateway-e32/devices/mailbox-sensor/messages/mailbox/distance`
* `lora-gateway-e32/devices/mailbox-sensor/messages/mailbox/humidity`
* `lora-gateway-e32/devices/mailbox-sensor/messages/mailbox/temperature`

Now you can easily:
* Add your own devices by:
    * Adding it to the `LORA_DEVICE_IDS` with an ID and name
* Add new message types by:
    * Define an ID for it
    * Define the struct that describes it

### How can I actually send messages?
Take a look at the [`sendLoRaMessage`](/src/lora-gateway-e32/lora-ids.h#L160) function:
`bool sendLoRaMessage(byte messageID, LoRaBase *loRaMessage, byte recipientId = 0, byte senderId = 0)`
It expects us to give it:
* The message ID we're going to send
* As also the message itself

We can also optionally specify:
* The recipientId of this message. If not specified, it defaults to `LORA_GATEWAY_ID`
* The senderId, which defaults to `LORA_DEVICE_ID`, if not specified

So in your code, simply create an instance of the message ID you want to send and pass it to `sendLoRaMessage`:
```
LoRaMessageMailbox *loRaMessage = new LoRaMessageMailbox;
loRaMessage->duration = duration;
loRaMessage->distance = distance;
loRaMessage->humidity = humidity;
loRaMessage->temperature = temperature;

sendLoRaMessage(LORA_MESSAGE_ID_MAILBOX, loRaMessage);
```
And that's it :)

#### How does it work under the hood?
For this we take a look at how the actual message is constructed that is sent via LoRa. The *basic idea* is stolen from the Arduino-LoRa library, which uses singly bytes to identify senders, receivers, etc.

Looking at a single message:
* 1st byte is the recipient ID
    * This can also be set to 0xFF, which indicates a broadcase message that is received and can be processed by all devices.
* 2nd byte is the sender ID
* 3rd byte is the message ID
* 4th-255th byte is the message we will split up
    * The message simply has all values joined together by a `|`. So taking the `mailbox` message example from above, the value for the message could look like this: `12345|3.56|44.55|27.4`
