/*
 
  Created by Jann Oesch (jann@oesch.org) 
  Copyright (c) 2022 Jann Oesch [JOE/TRS] 4A0354
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

#ifndef CH9350_USBKEY_h
#define CH9350_USBKEY_h

#include <Arduino.h>

class USB9350_KeyMouse 
{
  public:  
    void begin(Stream &serialport);  //Init, set Serial Port
    int GetKey();             //Get last pressed Key 
    uint16_t GetKeyRaw();     //Get last pressed Key Scancode  (8Bit Modifier|8Bit Scancode)
    int KeyPressed();         //Get actual pressed Key
    uint16_t KeyPressedRaw(); //Get actual pressed Key Scancode (8Bit Modifier|8Bit Scancode)
    int GetMouseXY(int *x, int *y, int *z, int *Button);  //Return MouseChange 0=No Cange since the last Call
    void SetMouseXY(int x, int y);                        //Set Mouse Coordinates 

  protected:
    char ScanToASCII(char mod, char code);   //Key Scancode and Modifier to ASCII
    void Check();                            //Check Serial and Create Key & Modifier

    char _MyKey, _MyMod;
    
    Stream *_SerPort;
    int _Startflag, _Pos, _CapsFlag, _NumFlag,_KeyPressed;
    char _SerArr[12];
    int _FirstMouseMove, _MouseX, _MouseY, _MouseZ, _MouseButton, _MouseXOff, _MouseYOff, _MouseChange;
    
};

#endif
