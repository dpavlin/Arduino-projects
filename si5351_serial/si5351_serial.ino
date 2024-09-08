/*
 * si5351_serial.ino - use Si5351Arduino to setup frequency over serial
 *
 * Copyright (C) 2024 Dobrica Pavlinusic <dpavlin@rot13.org>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * on Arduino Nano wire si5351 adafruit-like module to:
 * SCL - A5
 * SDA - A4
 * GND - GND
 * VCC - 5V
 */

#include "si5351.h"
#include "Wire.h"
#include <SerialCommands.h>
#include <EEPROM.h>

#define MAX_CHANNEL 2 // max. number of channels
// 2.5 = assumes 10Mhz reference instead od 25Mhz so all frequences
// will be multipled by 2.5, change to 1 if using stadard module
//#define FREQ_MUL 2.5
#define FREQ_MUL 1


// EtherKit Si5351 library
Si5351 si5351;

const byte eeprom_magic = 0x43;
bool eeprom_magic_ok = false;
float current_freq = 0.5; // Mhz
int channel = 0;
float freqs[MAX_CHANNEL+1] = { 1,2,3 }; // channel 0,1,2 frequencies

char serial_command_buffer_[32];
// SerialCommands library - end of line just CR
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\r", " ");

void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
  sender->GetSerial()->println("set frequency in Mhz: f 3.14");
  sender->GetSerial()->print("set channel 0-");
  sender->GetSerial()->print(MAX_CHANNEL);
  sender->GetSerial()->println(": c 1");
  sender->GetSerial()->println("status: s");
  sender->GetSerial()->println("write to eeprom: w");
  sender->GetSerial()->println("step (-/+ =) size change: step 0.5");
  sender->GetSerial()->println("reset from eeprom: r");
  
  sender->GetSerial()->print("channel=");
  sender->GetSerial()->print(channel);
  sender->GetSerial()->print(" current_freq=");
  sender->GetSerial()->print(current_freq, 5);
  sender->GetSerial()->println(" Mhz");
  

}


void set_clk_freq(uint8_t channel, float freq) {
  Serial.print("set_clk");
  Serial.print(channel);
  Serial.print("_freq ");
  Serial.println(freq, 5);
  freqs[channel] = freq;
  si5351.set_freq(100000000ULL * freq * FREQ_MUL, channel);
  print_status();
  
}

float step = 1;
void cmd_inc(SerialCommands* sender) { set_clk_freq(channel, freqs[channel] + step); }  
void cmd_dec(SerialCommands* sender) { set_clk_freq(channel, freqs[channel] - step); }

void cmd_step(SerialCommands* sender)
{
  char* f_str = sender->Next();
  if (f_str == NULL)
  {
    step = 1;
  } else {
    step = atof(f_str);
  }
}

//expects one single parameter
void cmd_f(SerialCommands* sender)
{
  //Note: Every call to Next moves the pointer to next parameter

  char* f_str = sender->Next();
  if (f_str == NULL)
  {
    sender->GetSerial()->println("ERROR no frequency");
    return;
  }

  float freq = atof(f_str);
  current_freq = freq;
  
  sender->GetSerial()->print("set frequency to ");
  sender->GetSerial()->println(freq, 5); // 5 decimal places

  set_clk_freq(channel, freq);  
}

void cmd_s(SerialCommands* sender)
{
  print_status();
}

void cmd_r(SerialCommands* sender) { read_eeprom(); }
void cmd_w(SerialCommands* sender)
{
  if (! eeprom_magic_ok) {
    EEPROM.put(0, eeprom_magic);
    eeprom_magic_ok = true;
  }
  for(int c=0; c<=MAX_CHANNEL; c++) {
    float f = freqs[c];
    EEPROM.put(1+(c*4), f); // each float is 4 bytes
  
    sender->GetSerial()->print("eeprom save of channel ");
    sender->GetSerial()->print(c);
    sender->GetSerial()->print(" frequency ");
    sender->GetSerial()->println(f, 5); // 5 decimal places
  }
}
void cmd_c(SerialCommands* sender)
{
  char* f_str = sender->Next();
  if (f_str == NULL)
  {
    sender->GetSerial()->println("ERROR no channel");
    return;
  }

  channel = atoi(f_str);
  if (channel > 2)
  {
    sender->GetSerial()->println("ERROR channel must be 0, 1 or 2");
    return;
  }
  
  sender->GetSerial()->print("channel ");
  sender->GetSerial()->println(channel);

}

#define SERIAL_COMMANDS_DEBUG 1


SerialCommand cmd_f_("f", cmd_f); /* freq */
SerialCommand cmd_s_("s", cmd_s); /* status */
SerialCommand cmd_w_("w", cmd_w); /* write */
SerialCommand cmd_r_("r", cmd_r); /* read/reset */
SerialCommand cmd_c_("c", cmd_c);
SerialCommand cmd_step_("step", cmd_step);
SerialCommand cmd_step2_("s", cmd_step);

// onekey, ative after last line terminator
SerialCommand cmd_inc_("=", cmd_inc, true);
SerialCommand cmd_inc2_("+", cmd_inc, true);
SerialCommand cmd_dec_("-", cmd_dec, true);



void setup()
{

  bool i2c_found;

  // Start serial and initialize the Si5351
  while( ! Serial ) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(1000);
    digitalWrite(LED_BUILTIN, HIGH);    
  }
  digitalWrite(LED_BUILTIN, LOW);
  Serial.begin(115200);
  i2c_found = si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  if(!i2c_found)
  {
    Serial.println("Device not found on I2C bus!");
  }
  Serial.println("setup");
  read_eeprom();
}

void read_eeprom() {
  Serial.println("read_eeprom");  
  byte eeprom_ok; 
  EEPROM.get(0, eeprom_ok);
  if (eeprom_ok == eeprom_magic) {
    eeprom_magic_ok = true;
    for(int c=0; c<=MAX_CHANNEL; c++) {
      float f;
      EEPROM.get(1 + (c*4), f);
      set_clk_freq(c, f);
      freqs[c] = f;
    } 

  } else {
    Serial.println("EEPROM data invalid, set frequency with 'f 1.234' in Mhz and issue w");
  }

  
  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_f_);
  serial_commands_.AddCommand(&cmd_s_);
  serial_commands_.AddCommand(&cmd_r_);
  serial_commands_.AddCommand(&cmd_w_);
  serial_commands_.AddCommand(&cmd_c_);

  serial_commands_.AddCommand(&cmd_inc_);
  serial_commands_.AddCommand(&cmd_inc2_);
  serial_commands_.AddCommand(&cmd_dec_);
  serial_commands_.AddCommand(&cmd_step_);
  serial_commands_.AddCommand(&cmd_step2_);

}

void loop()
{
  serial_commands_.ReadSerial();
}

void print_status()
{
  // Read the Status Register and print it every 10 seconds
  si5351.update_status();
  Serial.print("SYS_INIT: ");
  Serial.print(si5351.dev_status.SYS_INIT);
  Serial.print("  LOL_A: ");
  Serial.print(si5351.dev_status.LOL_A);
  Serial.print("  LOL_B: ");
  Serial.print(si5351.dev_status.LOL_B);
  Serial.print("  LOS: ");
  Serial.print(si5351.dev_status.LOS);
  Serial.print("  REVID: ");
  Serial.println(si5351.dev_status.REVID);
  
  Serial.print("freq 0=");
  Serial.print(freqs[0],5);
  Serial.print(" 1=");
  Serial.print(freqs[1],5);
  Serial.print(" 2=");
  Serial.print(freqs[2],5);
  Serial.print(" Mhz");

  Serial.print(" channel=");
  Serial.print(channel);
  Serial.print(" current_freq=");
  Serial.print(current_freq, 5);
  Serial.print(" Mhz ");
  Serial.print("step=");
  Serial.print(step);
  Serial.println();

}
