/* utils.ino -- Useful utilites.
 *  
 * Copyright (C) 2019 Artyom V. Poptsov <poptsov.artyom@gmail.com>
 *
 * This file is part of cadadrobot.
 *
 * cadadrobot is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * cadadrobot is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cadadrobot.  If not, see <http://www.gnu.org/licenses/>.
 */

String utils::read_line(File f) {
  String result = "";
  while (f.available()) {
    char ch = (char) f.read();
    if ((ch == '\n') || (ch <= 0)) {
      break;
    }
    result += ch;
  }
  return result;
}

File utils::open_file(const char* file_name, int mode) {
  File file = SD.open(file_name, mode);
  delay(100);
  if (! file) {
    Serial.println(F("Could not open file"));
  }
  return file;
}

//// utils.ino ends here.
