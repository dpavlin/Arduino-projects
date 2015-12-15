
/**************************************************************
*
*  Sample sketch for driving 32x16 LED PANEL (1/4 scan) with
*  HUB12 protocol
*  http://blog.vettore.org/building-a-large-led-sign-with-inexpensive-standard-modules-and-arduino/
*
*************************************************************/
 
#include <SPI.h>
 
//Pins specific for Mega .See Arduino SPI for a different board.
#define A 22 
#define B 24 
#define OE 26
#define R1 51
#define CLK 52
#define STB 53
 
 
//row to be shown (1-4 since it is 1/4 scan)
byte row=0; 
 
//brightness: increase->more bright
int br=500;
 
//some digits from a 8x8 font (numeric 1-8)
byte digits[]={
 
0x04,  //1
0x0C,
0x04,
0x04,
0x04,
0x04,
0x0E,
0x00,
 
0x0E, //2
0x11,
0x01,
0x02,
0x04,
0x08,
0x1F,
0x00,
 
 
0x1F, //3
0x02,
0x04,
0x02,
0x01,
0x11,
0x0E,
0x00,
 
0x02, //4
0x06,
0x0A,
0x12,
0x1F,
0x02,
0x02,
0x00,
 
0x1F, //5
0x10,
0x1E,
0x01,
0x01,
0x11,
0x0E,
0x00,
 
 
0x06, //6
0x08,
0x10,
0x1E,
0x11,
0x11,
0x0E,
0x00,
 
0x1F,//7
0x01,
0x02,
0x04,
0x04,
0x04,
0x04,
0x00,
 
 
0x0E,//8
0x11,
0x11,
0x0E,
0x11,
0x11,
0x0E,
0x00,
 
 
 
 
 
};
 
 
 
void setup () {
     pinMode(A, OUTPUT);
     pinMode(B, OUTPUT);
     pinMode(OE, OUTPUT);
     pinMode(R1, OUTPUT);
     pinMode(CLK, OUTPUT);
     pinMode(STB, OUTPUT);
     SPI.begin();
     delay(300); 
}    
 
 
//display alternatively scan lines
void loop(){
       showRow(0);
       showRow(1);
       showRow(2);
       showRow(3);
}
 
 
//Load and show row (1-4) i.e. 1 and 5, 2 and 6.....
void showRow(int row){
 
      SPI.transfer(~(digits[row+36]));   //5
      SPI.transfer(~(digits[row+32]));              
 
      SPI.transfer(~(digits[row+4]));    //1
      SPI.transfer(~(digits[row]));            
 
      SPI.transfer(~(digits[row+44]));   //6 
      SPI.transfer(~(digits[row+40]));              
 
      SPI.transfer(~(digits[row+12]));   //2
      SPI.transfer(~(digits[row+8]));              
 
      SPI.transfer(~(digits[row+52]));   //7
      SPI.transfer(~(digits[row+48]));              
 
      SPI.transfer(~(digits[row+20]));   //3
      SPI.transfer(~(digits[row+16]));              
 
      SPI.transfer(~(digits[row+60]));   //8
      SPI.transfer(~(digits[row+56]));              
 
      SPI.transfer(~(digits[row+28]));   //4
      SPI.transfer(~(digits[row+24]));              
 
      digitalWrite(STB,LOW);
      digitalWrite(STB,HIGH);
 
      scanrow(row);//enable encoder for the line loaded
 
      //PWM like. Change br to adjust brightnes
      digitalWrite(OE,HIGH);
      delayMicroseconds(br);
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
 

