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


char serial_command_buffer_[32];
SerialCommands serial_commands_(&Serial, serial_command_buffer_, sizeof(serial_command_buffer_), "\n", " ");

void cmd_unrecognized(SerialCommands* sender, const char* cmd)
{
  sender->GetSerial()->print("Unrecognized command [");
  sender->GetSerial()->print(cmd);
  sender->GetSerial()->println("]");
  sender->GetSerial()->println("set frequency in Mhz: f 3.14");
  sender->GetSerial()->println("status: s");
  sender->GetSerial()->println("write to eeprom: w");
  sender->GetSerial()->print("current_freq=");
  sender->GetSerial()->print(current_freq, 5);
  sender->GetSerial()->println(" Mhz");
  

}


void set_clk0_freq(float freq) {
  Serial.print("set_clk0_freq ");
  Serial.println(freq, 5);
  // * 2.5 since reference is 10 Mhz, not 25 Mhz
  si5351.set_freq(100000000ULL * freq * 2.5, SI5351_CLK0);
  print_status();
  current_freq = freq;
  
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
  
  sender->GetSerial()->print("set frequency to ");
  sender->GetSerial()->println(freq, 5); // 5 decimal places

  set_clk0_freq(freq);  
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
  EEPROM.put(1, current_freq);
  sender->GetSerial()->println("eeprom save of frequency ");
  sender->GetSerial()->println(current_freq, 5); // 5 decimal places
}


SerialCommand cmd_f_("f", cmd_f);
SerialCommand cmd_s_("s", cmd_s);
SerialCommand cmd_w_("w", cmd_w);



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
    EEPROM.get(1, current_freq); 
  } else {
    Serial.println("EEPROM data invalid, set frequency with 'f 1.234' in Mhz and issue w");
  }

  set_clk0_freq(current_freq);
  
  serial_commands_.SetDefaultHandler(cmd_unrecognized);
  serial_commands_.AddCommand(&cmd_f_);
  serial_commands_.AddCommand(&cmd_s_);
  serial_commands_.AddCommand(&cmd_w_);


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
  
}
