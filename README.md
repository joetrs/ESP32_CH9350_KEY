# ESP32 CH9350 KEY - add a USB keyboard to your controller 
Sometime it is useful to connect a Keyboard to a embedded controller. Long time ago I programmed a 8 pin PIC as interface from a ps2 keyboard to a serial interface. This days a good ps2 keyboard is hard to find and I still needed a solution. Then I found the CH9350 Chip witch does the interface between USB HID and a serial port. Unfortunatly the CH9350 give near - HID messages over the serial interface and not ASCII codes. I decided to create a simple library to do the translation and share it to the world. Hope you enjoy it.

This is a simple class for using a USB keyboard/mouse with a CH9350 chip or module.
Hope it's useful for you.
  
The included demo shows how to use it.

![alt text](https://github.com/joetrs/ESP32_CH9350_KEY/blob/main/IMG_7906.jpg?raw=true)

CH9350 should be in working mode "lower computer) (SEL = high)working status 4 (S0,S1 = low).
Look at the correct baudrate (b0,b1). For the demo both should be high (115200Baud)

![alt text](https://github.com/joetrs/ESP32_CH9350_KEY/blob/main/IMG_7907.jpg?raw=true)
  
