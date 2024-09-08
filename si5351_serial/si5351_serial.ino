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
 */

#include "si5351.h"
#include "Wire.h"
#include <SerialCommands.h>
#include <EEPROM.h>

Si5351 si5351;

const byte eeprom_magic = 0x42;
bool eeprom_magic_ok = false;
float current_freq = 0.5; // Mhz
int channel = 0;
#define MAX_CHANNEL 2
float freqs[MAX_CHANNEL+1]; // channel 0,1,2 frequencies

char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\n", " ");

void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
  sender->GetSerial()->println("set frequency in Mhz: f 3.14");
  sender->GetSerial()->print("set channel 0-");
  sender->GetSerial()->print(MAX_CHANNEL);
  sender->GetSerial()->print(": c 1");
  sender->GetSerial()->println("status: s");
  sender->GetSerial()->println("write to eeprom: w");
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
  // * 2.5 since reference is 10 Mhz, not 25 Mhz
  si5351.set_freq(100000000ULL * freq * 2.5, channel);
  print_status();
  
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



SerialCommand cmd_f_("f", cmd_f);
SerialCommand cmd_s_("s", cmd_s);
SerialCommand cmd_w_("w", cmd_w);
SerialCommand cmd_c_("c", cmd_c);



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
  serial_commands_.AddCommand(&cmd_w_);
  serial_commands_.AddCommand(&cmd_c_);


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
  Serial.println(" Mhz");
}
