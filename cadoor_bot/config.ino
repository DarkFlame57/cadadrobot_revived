/* config.ino -- Implementation of a configuration class.
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

#include "utils.h"

//// Methods.

Config::Config(String file_name) {
  this->file_name = file_name;
}

/**
 * Get an option by a KEY from the config file.
 * @return option as a String or an empty string if no option found.
 */
String Config::get(String key) {
  File conf = utils::open_file(this->file_name.c_str(), FILE_READ);
  while (conf.available()) {
    String line = utils::read_line(conf);
    size_t pos = line.indexOf(":");
    String opt = line.substring(0, pos);
    if (opt == key) {
      return line.substring(pos + 1);
    }
  }
  return "";
}

//// config.ino ends here.
