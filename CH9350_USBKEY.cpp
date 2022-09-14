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

#include "CH9350_USBKEY.h"

//Begin -> Init Class inkluding Serial Port
void USB9350_KeyMouse::begin(Stream &serialport)
{
    _SerPort = &serialport;
    _Startflag=0;
    _Pos=0;
    _CapsFlag=0;
    _NumFlag=0;
    _KeyPressed=0;

    _MyKey=0;
    _MyMod=0;
    
    _FirstMouseMove=0; //Set rollover mode the first time the mouse is moved
    _MouseX=0; _MouseXOff=0;
    _MouseY=0; _MouseYOff=0;
    _MouseZ=0;
    _MouseButton=0;
    _MouseChange=0;
}

//Check Keyboard and Mouse Move on CH9350 Serial Protocol
void USB9350_KeyMouse::Check()
{
    if(_SerPort->available())  //Serial data received?
      {
        int c=_SerPort->read();
        //Serial.printf("%02X",c);  //Debug

        if(c==0x57) _Startflag = 1;
        else if((c==0xAB) && (_Startflag == 1)) { _Startflag=0; _Pos=1;}
        else _Startflag = 0;
        
        if(_Pos>0)
        {
           if(_Pos < 12) _SerArr[_Pos-1]=c;
           _Pos++;
           if(_Pos==4)
           {
           if(_SerArr[1]==0x80) 
             {
                //Serial.printf("Status Change: %02X\n",_SerArr[2]);
                
                //Handling Caps Lock
                if(_CapsFlag==1)  //LED Caps on
                {
                   char buf[] = {0x57,0xAB,0x12,0x00,0x00,0x00,0x00,0x02,0x31,0x0F,0x20}; 
                   buf[8] = _SerArr[2];
                   buf[7] = 0x02 | (_NumFlag==2);
                   _SerPort->write(buf,11);
                   _CapsFlag=2; //Caps On
                }   
                else if(_CapsFlag>2)  //LED Caps off
                {
                   char buf[] = {0x57,0xAB,0x12,0x00,0x00,0x00,0x00,0x00,0x31,0x0F,0x20}; 
                   buf[8] = _SerArr[2];
                   buf[7] = 0x00 | (_NumFlag==2);
                   _SerPort->write(buf,11);
                   _CapsFlag=0; //Caps Off
                } 
                //Handling Num Lock
                else if(_NumFlag==1)  //LED Caps on
                {
                   char buf[] = {0x57,0xAB,0x12,0x00,0x00,0x00,0x00,0x01,0x31,0x0F,0x20}; 
                   buf[8] = _SerArr[2];
                   buf[7] = 0x01 | (_CapsFlag==2)<<1;
                   _SerPort->write(buf,11);
                   _NumFlag=2; //Num On
                }   
                else if(_NumFlag>2)  //LED Caps off
                {
                   char buf[] = {0x57,0xAB,0x12,0x00,0x00,0x00,0x00,0x00,0x31,0x0F,0x20}; 
                   buf[8] = _SerArr[2];
                   buf[7] = 0x00 | (_CapsFlag==2)<<1;
                   _SerPort->write(buf,11);
                   _NumFlag=0; //Num Off
                } 
                else
                {
                   //V2.6 does send status changes. I have no idea if a reaction is needed.
                   //Serial.printf("Unexpected Status Change: %02X\n",_SerArr[2]);
                   //char buf[] = {0x57,0xAB,0x12,0x00,0x00,0x00,0x00,0xFF,0x0F,0x00,0x20}; //Just give an answer
                   //buf[8] = _SerArr[2];
                   //_SerPort->write(buf,11);
                }
              
                _Pos=0;

             } 
           }
           if(_Pos==11) //Keyboard = 10 Byte
           {
             if(_SerArr[1]==0x01)
             {
                //Serial.printf("Keyboard Complete: Modifier: %02X, Keycode: %02X%02X\n",_SerArr[2],_SerArr[3],_SerArr[4]);

                if((_SerArr[3]==0x00)&&(_SerArr[4]==0x00)) //No Key pressed
                {
                   _KeyPressed=0;
                   _MyKey=0;
                }
                else //KeyPressed
                {
                  
                  if((_SerArr[4]==0x039)&&(_KeyPressed==0)) 
                  {
                    _CapsFlag++;  //Caps pressed, Keyboard will ask for LED Change
                  }
                  if((_SerArr[4]==0x053)&&(_KeyPressed==0)) 
                  {
                    _NumFlag++;  //Num Lock pressed, Keyboard will ask for LED Change
                  }
                  _MyKey=_SerArr[4];  //Key Scan Code
                  _MyMod=_SerArr[2];  //Key Modifier 
                  _KeyPressed=1;
                }
             }
           }  
           if(_Pos==10) //Mouse = 10 Byte
           {             
             if((_SerArr[1]==0x04) && (_SerArr[2]==0x01))
             {
                //Serial.printf("Mouse Complete: Button: %02X, X: %02X%02X Y: %02X%02X W:%02X\n",_SerArr[3],_SerArr[5],_SerArr[4],_SerArr[7],_SerArr[6],_SerArr[8]);
                int ActMouseX=((int)_SerArr[5]*256+_SerArr[4])&0x3FF;  //Range reduction to V2.3 of the CH9350 (compatibility)
                int ActMouseY=((int)_SerArr[7]*256+_SerArr[6])&0x3FF;  //Range reduction to V2.3 of the CH9350 (compatibility)
                //Serial.printf("Mouse Complete: Button: %02X, X: %04X Y: %04X% W:%02X\n",_SerArr[3],ActMouseX,ActMouseY,_SerArr[8]);

                //X Over/Underflow
                if((_MouseX-_MouseXOff)>(ActMouseX+512)) 
                {  
                   //Serial.printf("Mouse X Overflow\n");  //Last MouseX >>> ActMouseX = Overflow Positive
                   _MouseXOff=_MouseXOff+1024;
                   
                }
                if((_MouseX-_MouseXOff)<(ActMouseX-512)) 
                {  
                   //Serial.printf("Mouse X Underflow\n");
                   _MouseXOff=_MouseXOff-1024;
                }
                //Y Over/Underflow
                if((_MouseY-_MouseYOff)>(ActMouseY+512)) 
                {  
                   //Serial.printf("Mouse Y Overflow\n");  //Last MouseY >>> ActMouseY = Overflow Positive
                   _MouseYOff=_MouseYOff+1024;
                   
                }
                if((_MouseY-_MouseYOff)<(ActMouseY-512)) 
                {  
                   //Serial.printf("Mouse Y Underflow\n");
                   _MouseYOff=_MouseYOff-1024;
                }
                _MouseX=_MouseXOff+ActMouseX;
                _MouseY=_MouseYOff+ActMouseY;
                _MouseZ=(int)_SerArr[8];
                _MouseButton=(int)_SerArr[3];
                //Serial.printf("Mouse X: %d, Y: %d OX: %d OY:%d AX: %d AY:%d\n",_MouseX,_MouseY,_MouseXOff,_MouseYOff,ActMouseX,ActMouseY);
                // X,Y = 0...1024, W = -1/0/+1
                _MouseChange=1;
             }

             if(_FirstMouseMove==0)  //First time the mouse send data? Set mode to rollover 
             {
                char buf[] = {0x57,0xAB,0x12,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0F,0x20};
                _SerPort->write(buf,11);
                //Serial.printf("Mouse Set to rollover\n");
                //WCH CH9350 V2.6 Chip have large Mouse coordinates (more than 0x03FF). There is a comand to reduce to V2.3 range, but this work only in rollover.
                //I handle the problem now with a simple ANS 0x3FF in the code above 14-09-2022 JOE/TRS
                //char buf1[] = {0x57,0xAB,0x12,0x00,0x00,0x00,0x00,0xFF,0xFF,0x0A,0x20};  //WCH 9350 V2.6 - Mouse Coordinates new 0-0FFF and not 03FF. Make it the same.
                //_SerPort->write(buf1,11);
                _FirstMouseMove=1;
             }
             
           }           
           
        }
        //Serial.printf("Startflag = %d Pos = %d\n",_Startflag, _Pos);  //Debug
      }
}
char USB9350_KeyMouse::ScanToASCII(char mod, char code)
{
char RetKey=0;  //default is 0 (No key pressed)


// Keymap Tables: Feel free to modify for your Application. As there is no standard and I need only one byte valkues (no VT320 ESC sequences) I have defined years ago a Simple Map table
// See the pdf for details
//                          0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
char OE_BASE_KEYMAP[] =  { 0x00,0x00,0x00,0x00, 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',  // 0x
                            'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '1', '2',  // 1x
                            '3', '4', '5', '6', '7', '8', '9', '0',0x0D,0x1B,0x08,0x09, ' ', '-', '=', '[',  // 2x  ENTER, ESC, BACKSPACE, TAB
                            ']','\\',0xFF, ';','\'', '`', ',', '.', '/',0x02,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,  // 3x  CAPS = 2, F0-F6
                           0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0x1F,0xF0,0xFE,0x18,0x14,0x15,0x7F,0x17,0x16,0x13,  // 4x F7-F12, PrintScreen, ScrollLock, Pause, Insert, Home, PageUp, DelFwd,End, PageDown, Rightarrow
                           0x12,0x11,0x10,0x05, '/', '*', '-', '+',0x0D, '1', '2', '3', '4', '5', '6', '7',  // 5x Left, Down, Up, NumLock, Keypad Symbols
                            '8', '9', '0', '.',0xFF,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // 6x Unknown, Application, Not used symbols
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }; //7x Not used symbols

char OE_SHIFT_KEYMAP[] = { 0x00,0x00,0x00,0x00, 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L',  // 0x
                            'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '!', '@',  // 1x
                            '#', '$', '%', '^', '&', '*', '(', ')',0x0E,0x1B,0x08,0x0B,0x80, '_', '+', '{',  // 2x  Shift-ENTER, ESC, BACKSPACE, TAB, Shift-Space
                            '}', '|',0xFF, ':', '"', '~', '<', '>', '?',0x02,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,  // 3x  CAPS = 2, F0-F6
                           0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0x1F,0xF0,0xFE,0x18,0x14,0x15,0x7F,0x17,0x16,0x13,  // 4x  F7-F12, PrintScreen, ScrollLock, Pause, Insert, Home, PageUp, DelFwd,End, PageDown, Rightarrow
                           0x12,0x11,0x10,0x05, '/', '*', '-', '+',0x0D, '1', '2', '3', '4', '5', '6', '7',  // 5x  Left, Down, Up, NumLock, Keypad Symbols
                            '8', '9', '0', '.',0xFF,0x0A,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // 6x  Unknown, Application, Not used symbols
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }; //7x  Not used symbols

char OE_CTRL_KEYMAP[] =  { 0x00,0x00,0x00,0x00,0xA1,0xA2,0xA3,0xA4,0xA5,0xA6,0xA7,0xA8,0xA9,0xAA,0xAB,0xAC,  // 0x
                           0xAD,0xAE,0xAF,0xB0,0xB1,0xB2,0xB3,0xB4,0xB5,0xB6,0xB7,0xB8,0xB9,0xBA,0xE1,0xE2,  // 1x
                           0xE3,0xE4,0xE5,0xE6,0xE7,0xE8,0xE9,0xE0,0x0F,0x00,0x08,0x00, ' ',0x00,0x00,0x00,  // 2x  CTRL-ENTER, --, BACKSPACE, Space
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,  // 3x  F0-F6
                           0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,  // 4x  F7-F12 / Rightarrow
                           0x12,0x11,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // 5x  Left, Down, Up, NumLock, Keypad Symbols
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // 6x 
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }; //7x                            

char OE_ALT_KEYMAP[] =   { 0x00,0x00,0x00,0x00,0xC1,0xC2,0xC3,0xC4,0xC5,0xC6,0xC7,0xC8,0xC9,0xCA,0xCB,0xCC,  // 0x
                           0xCD,0xCE,0xCF,0xD0,0xD1,0xD2,0xD3,0xD4,0xD5,0xD6,0xD7,0xD8,0xD9,0xDA,0xBC,0xBD,  // 1x
                           0xBE,0xBF,0xDB,0xDC,0xDD,0xDE,0xDF,0xBB,0x0D,0x00,0x08,0x00, ' ',0x00,0x00,0x00,  // 2x  ENTER, --, BACKSPACE, Space
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,  // 3x  F0-F6
                           0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,  // 4x  F7-F12 / Rightarrow
                           0x12,0x11,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // 5x  Left, Down, Up, NumLock, Keypad Symbols
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // 6x 
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }; //7x 

char OE_ALTGR_KEYMAP[] = { 0x00,0x00,0x00,0x00,0x81,0x82,0x83,0x84,0x85,0x86,0x87,0x88,0x89,0x8A,0x8B,0x8C,  // 0x
                           0x8D,0x8E,0x8F,0x90,0x91,0x92,0x93,0x94,0x95,0x96,0x97,0x98,0x99,0x9A,0x9C,0x9D,  // 1x
                           0x9E,0x9F,0xEA,0xEB,0xEC,0xED,0xEE,0x9B,0x0D,0x00,0x08,0x00, ' ',0x00,0x00,0x00,  // 2x  ENTER, --, BACKSPACE, Space
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xF1,0xF2,0xF3,0xF4,0xF5,0xF6,  // 3x  F0-F6
                           0xF7,0xF8,0xF9,0xFA,0xFB,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x13,  // 4x  F7-F12 / Rightarrow
                           0x12,0x11,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // 5x  Left, Down, Up, NumLock, Keypad Symbols
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,  // 6x 
                           0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 }; //7x 

  if(mod==0x00)  //No modifier  
  {                     
     RetKey = OE_BASE_KEYMAP[code];  
     if(_CapsFlag==2)  //Caps a --> A
     {
        if((RetKey >= 'a') &&  (RetKey <= 'z'))
           RetKey=RetKey-32;
     }
  }   
  if((mod==0x02)||(mod==0x20))  //Left & Right Shift modifier
  {
     RetKey = OE_SHIFT_KEYMAP[code];
     if(_CapsFlag==2)  //Caps A --> a
     {
        if((RetKey >= 'A') &&  (RetKey <= 'Z'))
           RetKey=RetKey+32;
     }
  }

  if((mod==0x01)||(mod==0x10))  //Left & Right CTRL modifier
  {
     RetKey = OE_CTRL_KEYMAP[code];
  }

  if(mod==0x04)  //Left ALT modifier
  {
     RetKey = OE_ALT_KEYMAP[code];
  }

  if(mod==0x40)  //Right ALT (ALT GR) modifier
  {
     RetKey = OE_ALTGR_KEYMAP[code];
  }
                              
  return RetKey;
}     

//Returns once a pressed key. Key must be released for another notifying
int USB9350_KeyMouse::GetKey()
{

  char myKeyCode=0;
  
  Check(); //Check Serial Interface and if a Keycode / Mouse Code is received get it 

  if(_KeyPressed==1)
  {
    myKeyCode=ScanToASCII(_MyMod, _MyKey);
    char myKey; if(myKeyCode >= 0x20 and myKeyCode < 127) myKey=myKeyCode; else myKey=' ';
    //Serial.printf("ASCII Code: %02X '%c'\n",myKeyCode,myKey);
                    
    _KeyPressed=2;  //Key was requested by Application
  }

  return (int)myKeyCode;
  
}

uint16_t USB9350_KeyMouse::GetKeyRaw()
{
  uint16_t myKeyCode=0;
  
  Check(); //Check Serial Interface and if a Keycode / Mouse Code is received get it 

  if(_KeyPressed==1)
  {
    myKeyCode=(_MyMod<<8)+_MyKey;
    //Serial.printf("RAW Code: %04X\n",myKeyCode);                  
    _KeyPressed=2;  //Key was requested by Application
  }

   return (int)myKeyCode;
}

//Returns actual State of a pressed key on keyboard
int USB9350_KeyMouse::KeyPressed()
{

  char myKeyCode=0;
  
  Check(); //Check Serial Interface and if a Keycode / Mouse Code is received get it 

  myKeyCode=ScanToASCII(_MyMod, _MyKey);
  return (int)myKeyCode;
}

//Get actual pressed Key Scancode (8Bit Modifier|8Bit Scancode)
uint16_t USB9350_KeyMouse::KeyPressedRaw()
{
  uint16_t myKeyCode=0;
  
  Check(); //Check Serial Interface and if a Keycode / Mouse Code is received get it 

  myKeyCode=(_MyMod<<8)+_MyKey;
  return myKeyCode; 
}

int USB9350_KeyMouse::GetMouseXY(int *x, int *y, int *z, int *Button)
{
  Check(); //Check Serial Interface and if a Keycode / Mouse Code is received get it 
  
  *x=_MouseX;
  *y=_MouseY;
  *z=_MouseZ;
  *Button=_MouseButton;
  int rtn=_MouseChange;
  _MouseChange=0;
  return rtn;
}

void USB9350_KeyMouse::SetMouseXY(int x, int y)
{
   Check(); //Check Serial Interface and if a Keycode / Mouse Code is received get it 
   int dx=(_MouseX-_MouseXOff);
   int dy=(_MouseY-_MouseYOff);
   _MouseXOff = x - dx; _MouseX=x;
   _MouseYOff = y - dy; _MouseY=y;
}
