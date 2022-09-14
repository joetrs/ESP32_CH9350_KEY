/*
 
  Created by Jann Oesch (jann@oesch.org) 
  Copyright (c) 2022 Jann Oesch [JOE/TRS]
  Technology, Research & Software
  All rights reserved.

    This file is part of ESP32_CH9350_KEY

    ESP32_CH9350_KEY is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    ESP32_CH9350_KEY is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with ESP32_CH9350_KEY.  If not, see <http://www.gnu.org/licenses/>.
*/

//This example shows how to use the library
//14-09-2022 by Jann Oesch [JOE/TRS]

#include <HardwareSerial.h> //Serial Interfaces
#include "CH9350_USBKEY.h"


//IOs for CH9350 used on ESP32
#define RXKEY 39    
#define TXKEY 33  

//Globals
int KeyType;
unsigned long StartTime;



HardwareSerial SerialKey(2);   //Keyboard USB to Serial CH9350 @ Serial Interface 1
USB9350_KeyMouse USBKeyMouse;  //Create an Instance for Keyboard/ Mouse Decoder

void setup() 
{
    Serial.begin(115200);
    Serial.println("Start");

    SerialKey.begin(115200, SERIAL_8N1, RXKEY, TXKEY);
    USBKeyMouse.begin(SerialKey);

    Serial.printf("CH9350_USBKEY Demonstration\n");
    Serial.printf("14. September 2022 By Jann Oesch\n");
    Serial.printf("Press any key or move the mouse. Press space to change ASCII to RAW mode for keyboard\n");
    Serial.printf("Press left mousebutton to set the coordinate to 1000/1000 ant the right one for 0/0\n\n");

    KeyType=0; //ASCII-CODE
    StartTime=0;
}


void loop() 
{
    int key;
    uint16_t rawkey;
    char keyasc='*';

    if(KeyType==0)
    {
      key=USBKeyMouse.GetKey();
      if(key>0)
      {
         if((key>=0x20) and (key<=0x7e)) keyasc=(char)key;
         Serial.printf("Key 0x%02X '%c' pressed\n", key, keyasc);
         if(key==0x20) KeyType=1;  //Space toggle ASCII/OE-CODE to RAW 
         StartTime = millis();
      }
      else if(StartTime>0)
      {
        key=USBKeyMouse.KeyPressed();
        if(key==0x00) 
        {
          Serial.printf("Key released after %ldms\n",millis()-StartTime);
          StartTime=0;
        }
      }
    }
    else
    {
      rawkey=USBKeyMouse.GetKeyRaw();
      if(rawkey>0)
      {
         Serial.printf("RawKey: 0x%02X / Modifier: 0x%02X pressed\n", rawkey&0xFF, rawkey>>8);         
         if(rawkey==0x2C) KeyType=0;  //Space toggle ASCII/OE-CODE to RAW
         StartTime = millis();
      }
      else if(StartTime>0)
      {
        rawkey=USBKeyMouse.KeyPressedRaw();
        if(rawkey==0x00) 
        {
          Serial.printf("Key released after %ldms\n",millis()-StartTime);
          StartTime=0;
        }
      }
    }  
    
    int x,y,z,Button;
    int Change=USBKeyMouse.GetMouseXY(&x,&y,&z,&Button);
    if(Change) Serial.printf("Mouse X: %d, Y: %d Z: %d Button:%d\n",x,y,z,Button);

    if(Button==1) USBKeyMouse.SetMouseXY(0,0);
    if(Button==2) USBKeyMouse.SetMouseXY(1000,1000);
    
}

/*
 *     int GetKey();             //Get last pressed Key 
    uint16_t GetKeyRaw();     //Get last pressed Key Scancode  (8Bit Modifier|8Bit Scancode)
    int KeyPressed();         //Get actual pressed Key
    uint16_t KeyPressedRaw(); //Get actual pressed Key Scancode (8Bit Modifier|8Bit Scancode)
    int GetMouseXY(int *x, int *y, int *z, int *Button);  //Return MouseChange 0=No Cange since the last Call
    void SetMouseXY(int x, int y);                        //Set Mouse Coordinates 
*/    
