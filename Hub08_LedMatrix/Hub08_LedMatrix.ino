/**
 * LED Matrix library for http://www.seeedstudio.com/depot/ultrathin-16x32-red-led-matrix-panel-p-1582.html
 * The LED Matrix panel has 32x16 pixels. Several panel can be combined together as a large screen.
 *
 * Coordinate & Connection (Arduino -> panel 0 -> panel 1 -> ...)
 *   (0, 0)                                     (0, 0)
 *     +--------+--------+--------+               +--------+--------+
 *     |   5    |    4   |    3   |               |    1   |    0   |
 *     |        |        |        |               |        |        |<----- Arduino
 *     +--------+--------+--------+               +--------+--------+
 *     |   2    |    1   |    0   |                              (64, 16)
 *     |        |        |        |<----- Arduino
 *     +--------+--------+--------+
 *                             (96, 32)
 *
 */

#define FONT8x8 0

#include <avr/pgmspace.h>
#include "LEDMatrix.h"
#include "font.h"

#define WIDTH   64*1 // 2 panels * 64
#define HEIGHT  16

#if USE_SPI
#include <SPI.h> // Arduino IDE compilation won't work without this
#endif

// hub08 pinout: la lb lc ld en  r1  lat  clk            
//     LEDMatrix(a, b, c, d, oe, r1, stb, clk);
LEDMatrix matrix(4, 5, 6, 7, 9,  11, 10,  13);

// Display Buffer 128 = 64 * 16 / 8
uint8_t displaybuf[(WIDTH/8) * HEIGHT*2];

uint8_t displaybuf_w[((WIDTH/8)+1) * HEIGHT];

byte cell[16];

void MatrixWriteCharacter(int x,int y, char character)
{
  //Serial.print(x);
  //Serial.print("  ");
  //Serial.println(character);
#if FONT8x8
  for(int i=0; i<8; i++) {
     uint8_t *pDst = displaybuf_w + (i+y) * ((WIDTH / 8) + 1) + x  ;
     *pDst = (font_8x8[character - 0x20][i]);
  }
#else
  for(int i=0; i<16; i++) {
     cell[i] = pgm_read_byte_near(&font_8x16[(character - 0x20)][i]);
  }

  //uint8_t *pDst = displaybuf_w + (y) * ((WIDTH / 8) + 1) + x  ;
  uint8_t *pDst = displaybuf_w + x  + y * ((WIDTH/8)+1);

  byte mask = 1;
  for(int j=0; j<8; j++) {

    byte out  = 0;
    for(int i=0; i<8; i++) {
	out <<= 1;
	if ( cell[i] & mask ) {
		out |= 1;
        }
     }
     *pDst = out;
     pDst += (WIDTH/8)+1;

     mask <<= 1;
  }

  mask = 1;
  for(int j=0; j<8; j++) {
    byte out  = 0;
    for(int i=8; i<16; i++) {
	out <<= 1;
	if ( cell[i] & mask ) {
		out |= 1;
        }
     }
     *pDst = out;
     pDst += (WIDTH/8)+1;

     mask <<= 1;
  }
#endif
}



void matrixPrint(String c) {
  //c="33";
  //Serial.println(c);
  //Serial.println(c.length());
  for (int i=0 ; i<c.length() ; i++) {
      MatrixWriteCharacter(i,3,c[i]);
      matrix.scan();
      //Serial.print(i);
      //Serial.print(" -> ");
      //Serial.println(c[i]);
  }
}


void setup()
{
    matrix.begin(displaybuf, WIDTH, HEIGHT);
    Serial.begin(115200);
    matrix.clear();
    //matrixPrint("12345678");
    // uint8_t *pDst = displaybuf + y * (WIDTH / 8) + x / 8;
	memset(displaybuf_w, 0, sizeof(displaybuf_w));
}

void matrixDelay(int x) {
  for (int y=0; y<x; y++) { matrix.scan(); }
  
}

//String poruka="X       XX      XOX     XOOX    XOoOX   XOooOX  XOoIoOX XOoiioOX        ";
//String poruka="        Lorem ipsum dolor sit amet, consectetur adipiscing elit. Cras blandit libero id ex dapibus suscipit. Proin vitae cursus eros. Ut porttitor congue metus at viverra. In consectetur ex massa.";
String poruka="!\"#$%&'()*+,-./0123456789:;<=>?@AaBbCcDdEeFfGgHhIiJjKkLlMmNnOoPpQqReSsTtUuVvWwXxYyZz[\\]^_`{|}~";
//String poruka="~";

int pos = 0; // position in circular display

void loop()
{
    for (int p=0; p<poruka.length() ; p++) {
	pos = ( pos + 1 ) % (( WIDTH / 8 )+1);
//	int pos_ch = ( pos + (WIDTH/8) ) % (( WIDTH / 8 )+1);
	int pos_ch = ( pos + (WIDTH/8) ) % (( WIDTH / 8 )+1);
	MatrixWriteCharacter(pos_ch,0,poruka.charAt(p));

	int step_up = ((WIDTH/8)+1)-pos; // move up one line when falling off the end of circular buffer

	Serial.print(pos);
	Serial.print(" step_up=");
	Serial.print(step_up);
	Serial.print(" char=");
	Serial.println(poruka.charAt(p));

	for (int o=0; o<8; o++) {
		uint8_t *src  = displaybuf_w + pos;
		uint8_t *dest = matrix.offscreen_buffer();

		int i = 0;

		for (int y = 0; y < HEIGHT; y++ ) {
			for (int x = 0; x < (WIDTH/8); x++) {

				int j  = ( x   < step_up ? y : y-1 ) * ((WIDTH/8)+1) + x;
				int j1 = ( x+1 < step_up ? y : y-1 ) * ((WIDTH/8)+1) + x+1;

				*(dest + i) = ( *(src + j) << o ) | (( *(src + j1) & ( 0xff << 8 - o ) ) >> 8 - o );
//				*(dest + i) = *(src + j);
#if USE_SPI
				matrix.scan();
				delayMicroseconds(100 / (WIDTH / 64));
#else
				matrix.scan();
				delayMicroseconds(100 / (WIDTH / 64));
#endif
				i++;
			}
                }
                matrix.swap();
          }
    }

}
