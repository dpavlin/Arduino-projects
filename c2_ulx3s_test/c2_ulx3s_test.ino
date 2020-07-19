#include <Compositing.h>
#include "font.h"
#include "rtc.h"
#include "adc.h"
#include "dac.h"
#include "edid.h"
#include "btn.h"
#include "flash.h"
#include "sd.h"
#include "oled.h"
#include "ram.h"
#include "rds.h"
#include "pcm.h"

#define N_LETTERS (sizeof(Font)/sizeof(Font[0]))

Compositing c2;

const int Cols = 40;
const int Rows = 30;
int Cursor_col = 0, Cursor_row = 0;

void cls()
{
  for(int row = 0; row < Rows; row++)
    for(int col = 0; col < Cols; col++)
    {
      int i = N_LETTERS + row*Cols + col;
      c2.sprite_link_content('@'-'@', i);
    }
  Cursor_col = Cursor_row = 0;
}

void prints(char *a)
{
  for(;*a != '\0'; a++)
  {
    int i = N_LETTERS + Cursor_row*Cols + Cursor_col;
    uint8_t letter = 0; // space is default
    char c = *a;
    if(c == '\n')
    {
      Cursor_col = 0;
      Cursor_row = (Cursor_row + 1) % Rows;
      continue;
    }
    c = toupper(c);
    if(c >= ' ' && c <= '_')
      letter = c - ' ';
    c2.sprite_link_content(letter, i);
    if(++Cursor_col >= Cols)
    {
      Cursor_col = 0;
      Cursor_row = (Cursor_row + 1) % Rows;
    }
  }  
}

// initialize compositing2 text using sprites
void video_init()
{
  int i;
  int unique_sprites;
  c2.init();
  c2.alloc_sprites(N_LETTERS+Rows*Cols); // it sets c2.sprite_max
  *c2.videobase_reg = NULL; // disable video during update
  *c2.cntrl_reg = 0;
  #if 1
    for(i = 0; i < c2.sprite_max && i < N_LETTERS; i++)
      c2.shape_to_sprite(&(Font[i]));
    unique_sprites = c2.n_sprites;
    // unique sprites containing alphabet are not displayed, set them off-screen
    for(i = 0; i < unique_sprites; i++)
    {
      c2.Sprite[i]->x = -100;
      c2.Sprite[i]->y = -100;
    }
    // position cloned sprites on display
    // i = N_LETTERS;
    for(int row = 0; row < Rows; row++)
      for(int col = 0; col < Cols; col++)
    {
      c2.sprite_clone(0); // by default clone 0-sprite which is space
      int n = N_LETTERS + row*Cols + col;
      c2.Sprite[n]->x = col*16;
      c2.Sprite[n]->y = row*16;
      i++;
    }
  #endif
  // cls();
  c2.sprite_refresh();
  // this is needed for vgatext
  // to disable textmode and enable bitmap
  *c2.cntrl_reg = 0b11000000; // enable video, yes bitmap, no text mode, no cursor
}

void setup()
{
  Serial.println("setup start");
  video_init();
  rtc_init();
  adc_init();
  dac_init();
  btn_init();
  oled_init();
  rds_init();
  pcm_init();
  rtc_set_clock();
  rtc_set_alarm();
  Serial.println("setup end");
}

int rtc_ok = 0;
int edid_ok = 0;
int adc_ok = 0;
int dac_ok = 0;
int btn_ok = 0;
int flash_ok = 0;
int sd_ok = 0;

int all_ok = 0;
int last_all_ok = -1;

int led_beat = 0;


void loop()
{
  static uint8_t counter = 0;
  const int nlines = 6;
  char line[nlines][256];

  digitalWrite(8+7, led_beat);
  led_beat = !led_beat;

#if 0
  if(1 & ++counter)
    pcm_tone();
  else
    pcm_mute();
#endif

  if ( rtc_ok == 0 ) rtc_ok = rtc_read(line[0]);
  if ( rtc_ok == 1 ) {
    Serial.println("RTC OK");
    rtc_ok++;
    all_ok++;
  }
  if ( edid_ok == 0 ) edid_ok = edid_read(line[1]);
  if ( edid_ok == 1 ) {
    Serial.println("EDID OK");
    edid_ok++;
    all_ok++;
  }
  if ( adc_ok < 8 ) adc_ok = adc_read(line[2]);
  if ( adc_ok == 8 ) {
    Serial.println("ADC OK");
    adc_ok++;
    all_ok++;
  }
  if ( dac_ok == 0 ) dac_ok = dac_read(line[3]);
  if ( dac_ok == 1 ) {
    Serial.println("DAC OK");
    dac_ok++;
    all_ok++;
  }
  
  int btn = btn_read(line[4]); // buttons. DIP switches and blink LEDs
  if ( btn_ok == 0 && btn == 7 ) {
    Serial.println("BTN OK");
    btn_ok++;
    all_ok++;
  }

#if 0
  if(line[4][5] == '1') // BTN1 pressed
  {
    rtc_set_clock();
    rtc_set_alarm();
    oled_init();
  }
  if(line[4][6] == '2') // BTN2 pressed - shutdown
  {
    volatile uint32_t *simple_out = (uint32_t *)0xFFFFFF10;
    simple_out[0] |= (1<<13); // bit 13 of simple_out is shutdown
  }
#endif
  char flash_str[64], sd_str[64], oled_str[64];
  if ( flash_ok == 0 ) flash_ok = flash_read(flash_str);
  if ( flash_ok == 1 ) {
    Serial.println("FLASH OK");
    flash_ok++;
    all_ok++;
  }
  
  if ( sd_ok == 0 ) sd_ok = sd_read(sd_str); // esp32 must be flashed not to access SD card
  if ( sd_ok == 1 ) {
    Serial.println("SD OK");
    sd_ok++;
    all_ok++;
  }

#if 0
  oled_read(oled_str);
#endif

  Serial.print("all_ok=");
  Serial.println(all_ok);

  sprintf(line[5], "%s %s   %s\n", flash_str, oled_str, sd_str);
  line[6][0]='\0';
  //ram_test(line[5]); // works but too slow, need speedup
  Serial.println("");
  for(int i = 0; i < nlines; i++)
    Serial.print(line[i]);

  cls();
  for(int i = 0; i < nlines; i++)
    prints(line[i]);
  
  while((*c2.vblank_reg & 0x80) == 0);
  c2.sprite_refresh();
  // delay(1000);
}
