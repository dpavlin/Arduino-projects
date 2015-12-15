
/**************************************************************
*
*  Sample sketch for driving 24x16 LED PANEL (1/4 scan) with
*  HUB12 protocol
*  Support for multiple cascaded panels
*  Get text TO DISPLAY from the serial monitor
*  http://blog.vettore.org/building-a-large-led-sign-with-inexpensive-standard-modules-and-arduino/
*
*************************************************************/
 
#include <SPI.h>
#include <Timer.h>
#include "font.h"
Timer t;
Timer t1;
 
//Pins specific for Mega.See Arduino SPI for a different board.
#define A 2 
#define B 3 
#define OE 4
#define R1 51
#define CLK 52
#define STB 53
 
//number of cascaded panels  
#define N_PANELS 4  
 
//row to be shown (1-4 since it is 1/4 scan)
byte row=0; 
 
//brightness: increase->more bright
int br=400;
 
int incomingByte = 0;
 
//buffer 8 characters for N_PANELS
 
 
 
byte stringDisp[8*N_PANELS]={
  'a','b','c','d','e','f','g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v','x','y','0','1','2','3','4','5','6','7',
};
 
 
 
//pixBuffer reserved space for N_PANELS
//contains status of every single pixel in our display!
byte pixBuffer[8*8*N_PANELS]={
};
 
 
 
void setup () {
     Serial.begin(9600);
     pinMode(A, OUTPUT);
     pinMode(B, OUTPUT);
     pinMode(OE, OUTPUT);
     pinMode(R1, OUTPUT);
     pinMode(CLK, OUTPUT);
     pinMode(STB, OUTPUT);
 
     SPI.begin();
 
     //set timers
     //first timer is the refresh rate for the 1/4 scan
     t.every(6,display);
     //second timer is for change display text IF INPUT AVAILABLE
     t1.every(1000,checkInput);
 
     delay(300); 
}    
 
 
void loop(){
 
  t.update();
  t1.update();
 
}
 
 
//diplay alternatively the 4 scan lines
void display(){
       showRow(0);
       showRow(1);
       showRow(2);
       showRow(3); 
}
 
 
 
//Load and show row (1-4) i.e. 1-5-9-13, 2-6-10-14.....
void showRow(int row){  
 
      int col=0;//is a column of 2 chars
 
      for(col=0;col<4*N_PANELS;col++){//show 2 characters every cicle
        SPI.transfer(~(pixBuffer[row+(col*8)+4+(32*N_PANELS)])); 
        SPI.transfer(~(pixBuffer[row+(col*8)+(32*N_PANELS)]));
        SPI.transfer(~(pixBuffer[row+(col*8)+4]));
        SPI.transfer(~(pixBuffer[row+(col*8)]));
      }
 
      digitalWrite(STB,LOW);
      digitalWrite(STB,HIGH);
 
      scanrow(row);//enable encoder for the line loaded
 
      //PWM like. Change br to adjust brightnes
      digitalWrite(OE,HIGH);
      delayMicroseconds(br);//PWM per aggiustare luminosità
      digitalWrite(OE,LOW);
      delayMicroseconds(900);
}
 
//enable encoder for this row in order to show it
void scanrow(int r){
    if(r==0){
      digitalWrite(A,0);
      digitalWrite(B,0);
    }  
    else if(r==1){
      digitalWrite(A,1);
      digitalWrite(B,0);
    }
    else if(r==2){
      digitalWrite(A,0);
      digitalWrite(B,1);
    }
    else if(r==3){
      digitalWrite(A,1);
      digitalWrite(B,1);
    }
}
 
//check if input from the serial monitor is available 
//and update the display
void checkInput(void){
  int x=0;
  int y=0;
  if(Serial.available()){
        int h;
        //clean display
        for(h=0;h<(8*N_PANELS);h++)stringDisp[h]=32;
  }
  while (Serial.available() > 0) {
           // read the incoming byte:
           incomingByte = Serial.read();//only UP to the efective string length
           if(x<(8*N_PANELS)) stringDisp[x]=incomingByte;
           x++; 
          }
  loadBuffer();
}
 
 
 
//load buffer with character pixels of the string
//performing a lookup on the FONT table
void loadBuffer(void){  
      int x;
      int y;
      for(y=0;y<(8*(N_PANELS));y++){            
           for(x=0;x<8;x++){
               pixBuffer[x+8*y]=font[8*stringDisp[y]+x-(31*8)];//char under <=31 not defined
            }
      }
}

