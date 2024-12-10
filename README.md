# Project Aim
This project is a man in the middle interface to retrofit to a Rock-ola 477 dukebox. It has a number of intended functions mainly:

* To replace the VFDs of the machine with 7 Segment LEDs
* To allow the record select buttons to control a software dukebox running fruitbox
* To allow the switching of audio sources from the Dukeboxe's own vinyl records to audio output of fruitbox
* To control the mute function of the Rock-ola to enable the amplifier to play other audio sources.

Additionally there may be future plans to add bluetooth connectivity

The code is developed on platformio using Arduino libraries.

# Hardware

![WhatsApp Image 2024-09-02 at 18 16 52_704379d6](https://github.com/user-attachments/assets/6f50a658-8f9f-423a-b2a5-7cf178b2ba62)

The hardware is an Arduino Pro-Micro microcontroller atteached to a Raspberry Pi. Using off the shelf modules the "Bo-Selecta" board uses electrical relays to mimic the button presses from the Rock-ola's selector buttons, a PCF8575 IO expander gives additional output pins for this purpose.  

The original buttons are in turn read by the Arduino Pro-Micro via a PCF8575 IO expander.

Button selections are sent to a 7 segment LED module to emulate the (very often broken) VFD of the dukebox. 

As of 10/12/2024 the Button selection is working, the relay board (with the addition of 1N4148 diodes) is interfacing with the Rockola and the Pro-Micro is behaving as a keyboard for button selections designed for Fruitbox V2
