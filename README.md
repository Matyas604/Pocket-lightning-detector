# Pocket lightning detector
A while back, I was browsing websites with various components and modules when a lightning sensor cought my eye. I was really curious about how it works, so I started researching it. After watching a few Youtube videos 
about this module, I decided I had to try it out. Then I started thinking, and suddenly occurred to me that it would be really cool to create a small, portable, battery-powered device that fits in your pocket and detect 
lightnings. So why not? Why not just design my own PCB and build the entire device myself?

I really enjoyd this project because I tried out a lot of different thigs: studying datasheets and schematics, searching for and creating my own schematic, and designing the PCB and case.

## Features
- The entire device is powered by a **Li-Po** rechargeable battery
- It uses the [**ESP-C3-MINI**](https://documentation.espressif.com/esp32-c3-mini-1_datasheet_en.pdf) as the main microcontroller
- Has **USB-C** port for programming and charging the device
- [**AS3935**](https://www.laskakit.cz/user/related_files/as3935_datasheet.pdf) and **MA5532-AE** for lightning detection
- Detects lightning strikes up to **40 km** away
- Records the distance, energy and current time of lightning strike
- Everything is stored in the internal **RTC memory**, which can store up to 50 lightnings strikes
- Communicates with a mobile via **Bluetooth** to synchronize the time and data
- Has audio feedback when a lightning strike been detected or other thing happened
- Various battery protections, such as **overcharge**, **undercharge**, **over-discharge** and **overvoltage**
- It uses deep sleep mode to get the longest possible battery life

## CAD model
Everything is stored in a case consisting of two parts: a base and a top cover. The top cover is secured with four **M3** screws and heat inserts, along with the **PCB**, which is secured with two screws. The buttons are labeled
with icons for clear identification of their functions. The dimensions are 106 mm in length, 60 mm in width, and 35 mm in height.

<p align="center">
  <img src="Assets/CAD.png" alt="CAD model" width="700">
</p>
<p align="center">
  <strong>Made in Fusion360</strong>
</p>

## PCB
It includes two switches: one to turn on and of the device and other to activate **Bluetooth**. In the upper right corner, above the **USB-C** port, are chips for charging, protection, monitoring, and drawing power from the battery, there is also a connector for the battery. The lightning sensor communicates via **I2C**. The pull-up resistors are located under the **ESP-32** near the edge of the **PCB**. The dimensions of **PCB** are 99.7 mm x 35 mm.

<p align="center">
  Schematic
</p>
<p align="center">
  <img src="Assets/Schematic.png" alt="PCB schematic" width="900">
</p>
<p align="center">
  PCB
</p>
<p align="center">
  <img src="Assets/PCB.png" alt="PCB" width="1000">
</p>
<p align="center">
  <strong>Made in KiCad</strong>
</p>
⚠️ The area around the lightning sensor antenna must be kept sufficiently far away from metal parts, as there could interfere with the antenna. For this reason, there si no GND copper area around it, and no screws are located nerby.

## Firmware
It uses standard **C++** code. One button turns the device on, and the other activates **Bluetooth** to display lightnings data and synchronize the time. The time must be synchronized every time the device is turned on. Data cen be currently displayed via a serial monitor application. Once the device is connected, it automatically sends lightning data every 3 seconds. Time is synchronized by sending a command, and the board then retrieves the current time. Lightning strikes and the time are stored in **RTC memory**, so they are erased when the device is turned off. The device runs continuously in deep sleep mode and wakes up only when a lightning strike occurs to save data about the strike and the time it happened, then goes back into deep sleep. **RTC memory** is the only component that remains active even when the device is in deep sleep mode. The **RTC memory** stores a maximum of the last **50 lightning strikes** to prevent it from becoming overloaded. My goal was to maximize battery life, so I chose **Bluetooth** over **Wi-Fi** because **Bluetooth** is much more power-efficient, and I use deep sleep mode as well (Once the device is finished, I’ll carefully measure its battery life).  You can found further details in the comments within the code. After i finished the device, I’ll probably create a nice app where the lightnings will be saved and can be viewed back in time. 

## BOM
You can find it in .csv format [here](https://github.com/Matyas604/Pocket-lightning-detector/blob/main/BOM.csv). 

<hr>

<p align="center">
  Made with ❤️ by <a href="https://github.com/Matyas604">@Matyas604</a>
</p>
