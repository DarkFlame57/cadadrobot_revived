/* cadoor_bot -- Hackerspace door bot.

   Copyright (C) 2019 Artyom V. Poptsov <poptsov.artyom@gmail.com>

   This file is part of cadadrobot.

   cadadrobot is free software: you can redistribute it and/or
   modify it under the terms of the GNU General Public License as
   published by the Free Software Foundation, either version 3 of the
   License, or (at your option) any later version.

   cadadrobot is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with cadadrobot.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <SPI.h>
#include <SD.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WiFiUdp.h>

#include <time.h>

#include "utils.h"

// Bot configuration
#include "config.h"

extern "C" {
#include "user_interface.h"
}

#define DEBUG true

const char* WHITELIST_FILE = "WL.TXT";
const char* LOG_FILE = "LOG.TXT";

WiFiClientSecure client;
Config config("CONF.TXT");
UniversalTelegramBot* bot;
File whitelist;

const int LOCK_PIN    = 5;
const int BUTTON_PIN  = 0;
const int SPEAKER_PIN = 16;

void play_tone(int pin, float f, long len) {
  int p = 1000000 / f;
  int d = p / 2;
  int count = len / p;
  for (int c = 0; c < count; c++) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(d);
    digitalWrite(pin, LOW);
    delayMicroseconds(d);
  }
}

void init_sdcard() {
  Serial.print(F("Initializing SD card..."));
  pinMode(4, OUTPUT);
  pinMode(4, HIGH);

  while (! SD.begin(4)) {
    Serial.println(F("initialization failed!"));

    delay(1000);
    ESP.reset();
    delay(10000);
  }
  Serial.println(F("initialization done."));
}

void open_file(int mode) {
  whitelist = utils::open_file(WHITELIST_FILE, mode);
}

void close_file() {
  whitelist.flush();
  whitelist.close();
}

void log(String date, String msg) {
  File logf = SD.open(LOG_FILE, FILE_WRITE);
  if (! logf) {
    Serial.println("[error] Could not open log file.");
    return;
  }
  time_t now = date.toInt() + 3600 * 3;
  String ts = ctime(&now);
  ts = ts.substring(0, ts.length() - 1);
  logf.println(String("") + ts + " " + msg);
  logf.close();
}

bool is_button_pressed() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    if (DEBUG) {
      Serial.println(F("[debug] button pressed"));
    }
    return true;
  }
  return false;
}

void open_door() {
  digitalWrite(LOCK_PIN, HIGH);
  play_tone(SPEAKER_PIN, 291.63, 500000);
  digitalWrite(LOCK_PIN, LOW);
}

void wait_wifi() {
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if (is_button_pressed()) {
      open_door();
    }
  }
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(500);
  }

  pinMode(LOCK_PIN, OUTPUT);
  digitalWrite(LOCK_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT);
  pinMode(SPEAKER_PIN, OUTPUT);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  Serial.println(F("WIFI disconnected"));
  delay(100);
  Serial.println(F("Initializing SD card..."));
  init_sdcard();
  Serial.println(F("done"));
  delay(100);

  String ssid = config.get("ssid");
  String password = config.get("password");
  Serial.print(F("Connecting Wifi: "));
  Serial.println(ssid);
  IPAddress ip(192, 168, 42, 15);
  IPAddress gateway(192, 168, 42, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns1(8, 8, 8, 8);
  WiFi.config(ip, gateway, subnet, dns1);
  WiFi.begin(ssid.c_str(), password.c_str());
  wait_wifi();

  Serial.println(F("\nWiFi connected"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());
  if (DEBUG)
    Serial.println(F("Creating a bot..."));
  bot = new UniversalTelegramBot(config.get("token"), client);
  delay(1000);
  Serial.println(F("Starting..."));
}

void send_error_unauthorized(String chat_id) {
  bot->sendMessage(chat_id, "not authorized", "");
}

void send_ok(String chat_id) {
  bot->sendMessage(chat_id, "ok", "");
}

/**
   Check if an user is in the whitelist by their ID.
   @returns true if ID is present in the whitelist, false otherwise.
*/
static bool is_user_in_wl(String id) {
  open_file(FILE_READ);
  while (whitelist.available()) {
    String line = utils::read_line(whitelist);
    if (line.indexOf(id) >= 0) {
      return true;
    }
    delay(100);
  }
  return false;
  close_file();
}

bool is_authorized(String date, String id, String name) {
  bool result = false;
  if (DEBUG)
    Serial.println(F("[debug] is_authorized"));

  if (is_user_in_wl(id)) {
    result = true;
    log(date, "user with ID " + id + " (" + name + ") is authorized");
  } else if (DEBUG) {
    Serial.println(F("[debug] is_authorized: not authorized"));
  }

  return result;
}

void handle_cmd_logtail(String chat_id, int count) {
  if (DEBUG)
    Serial.println("[debug] handle_cmd_logtail");

  if (count <= 0) {
    bot->sendMessage(chat_id,
                     "Bzzzt.  The parameter should be > 0",
                     "");
    return;
  }

  File logf = SD.open(LOG_FILE, FILE_READ);
  uint32_t line_count = 0;
  while (logf.available()) {
    if (logf.read() == '\n') {
      ++line_count;
    }
  }
  logf.seek(0);
  String response = "";
  for (int cnt = 0; logf.available(); cnt++) {
    if (cnt > (line_count - 10)) {
      response += utils::read_line(logf) + "\n";
    } else {
      utils::read_line(logf);
    }
  }
  logf.close();
  bot->sendMessage(chat_id, response, "");
}

void handle_open_door(String chat_id) {
  if (DEBUG)
    Serial.println("[debug] handle_open_door");
  open_door();
  send_ok(chat_id);
}

const char* CMD_OPEN    = "/open";
const char* CMD_STATUS  = "/status";
const char* CMD_USERADD = "/useradd";
const char* CMD_USERDEL = "/userdel";
const char* CMD_USERLS  = "/userls";
const char* CMD_HELP    = "/help";
const char* CMD_ID      = "/id";
const char* CMD_LOGTAIL = "/logtail";

String get_uptime() {
  long millisecs = millis();
  String result;
  result += int(millisecs / (1000.0 * 60.0 * 60.0 * 24.0)) % 365;
  result += String(":") + int((millisecs / (1000.0 * 60.0 * 60.0))) % 24;
  result += String(":") + int(millisecs / (1000.0 * 60.0)) % 60;
  return result;
}

void handle_cmd_status(String chat_id) {
  String response = "";
  response += String("Uptime:   ") + get_uptime() + "\n";
  response += String("Free RAM: ") + system_get_free_heap_size() + "\n";
  open_file(FILE_READ);
  if (whitelist) {
    response += "SD card:\tok";
  } else {
    response += "SD card:\tnot connected";
  }
  close_file();
  bot->sendMessage(chat_id, response, "");
}

void handle_cmd_userls(String chat_id) {
  String response = "";
  int counter = 0;

  open_file(FILE_READ);

  for (int idx = 0; whitelist.available();) {
    String line = utils::read_line(whitelist);
    if (line.length() > 0) {
      response += String("") + idx + ". " + line + "\n";
      ++idx;
    }
  }

  close_file();

  bot->sendMessage(chat_id, response, "");
}

void handle_cmd_useradd(String chat_id, String user_id,
                        String user_name) {
  Serial.println(String("user_id: '") + user_id + "'");
  if (user_id.length() == 0) {
    bot->sendMessage(chat_id, "Could not add empty ID", "");
    return;
  }
  if (is_user_in_wl(user_id)) {
    bot->sendMessage(chat_id,
                     "Bzzt.  "
                     "User with the given ID is already in the list",
                     "");
    return;
  }
  open_file(FILE_WRITE);
  whitelist.print(String("\n") + user_id + ":" + user_name);
  whitelist.flush();
  close_file();
  send_ok(chat_id);
}

boolean file_mv(String src, String dst) {
  File src_f = SD.open(src, FILE_READ);
  if (! src_f) {
    Serial.println("[warning] file_mv: Could not open source file.");
    return false;
  }
  SD.remove(dst);
  File dst_f = SD.open(dst, FILE_WRITE);
  if (! dst_f) {
    Serial.println("[error] file_mv: Could not open destination file.");
    return false;
  }
  while (src_f.available()) {
    String line = utils::read_line(src_f);
    Serial.println(line);
    dst_f.println(line);
  }
  src_f.close();
  SD.remove(src);
  dst_f.close();
  return true;
}

void handle_cmd_userdel(String chat_id, String caller_id, String user_id) {
  const char* TMP_FILE = "wltmp.txt";
  Serial.println(chat_id + ", " + caller_id + ", " + user_id);
  if (! is_user_in_wl(user_id)) {
    bot->sendMessage(chat_id, "Bzzzt.  No such user in the whitelist.", "");
    return;
  }
  if (caller_id == user_id) {
    bot->sendMessage(chat_id, "Bzzzt.  You cannot delete yourself.", "");
    return;
  }
  open_file(FILE_READ);
  File tmp = SD.open(TMP_FILE, FILE_WRITE);
  String line;

  if (whitelist.available()) {
    // Skip the fist line to protect the "super-admin".
    line = utils::read_line(whitelist);
    tmp.println(line);
    if (DEBUG) {
      Serial.print(F("[debug] handle_cmd_userdel: Skipped line: "));
      Serial.println(line);
    }
  }

  while (whitelist.available()) {
    line = utils::read_line(whitelist);
    if (DEBUG) {
      Serial.print(F("[debug] handle_cmd_userdel: Read line: "));
      Serial.println(line);
    }
    if ((line.indexOf(user_id) < 0) && (line.length() > 2)) {
      tmp.println(line);
    }
  }
  tmp.close();
  close_file();

  if (! file_mv(TMP_FILE, WHITELIST_FILE)) {
    bot->sendMessage(chat_id, "Bzzzt.  Failed to update whitelist.", "");
  } else {
    send_ok(chat_id);
  }
}

String get_token(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if ((data.charAt(i) == separator) || (i == maxIndex)) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

void handle_cmd_help(String chat_id) {
  String result = "Available commands:\n";
  result += "* /open    -- open the door\n";
  result += "* /userls  -- list available users\n";
  result += "* /useradd <user-id> <user-name> -- add a new user\n";
  result += "* /userdel <user-id> -- remove a user with the given ID\n";
  result += "* /status  -- show the system status\n";
  result += "* /id      -- print your ID\n";
  result += "* /logtail [count] -- show the last COUNT lines of the log file.";
  result += " If COUNT is not specified, show the last 10 lines.\n";
  result += "* /help    -- print this message\n";
  bot->sendMessage(chat_id, result, "");
}

void handle_cmd_help_user(String chat_id) {
  String result = "Available commands:\n";
  result += "/id      -- print your ID\n";
  result += "/help    -- print this message\n";
  bot->sendMessage(chat_id, result, "");
}

void handle_cmd_id(String chat_id, String from_id) {
  bot->sendMessage(chat_id, from_id, "");
}

void handle_messages() {
  if (DEBUG)
    Serial.println(F("[debug] requesting bot updates ..."));
  int message_count = bot->getUpdates(bot->last_message_received + 1);
  while (message_count) {
    Serial.print(F("Got messages: "));
    Serial.println(message_count);
    for (int i = 0; i < message_count; i++) {
      String chat_id   = bot->messages[i].chat_id;
      String from_id   = bot->messages[i].from_id;
      String from_name = bot->messages[i].from_name;
      String msg_text  = bot->messages[i].text;
      String date      = bot->messages[i].date;
      if (msg_text == CMD_OPEN) {
        if (is_authorized(date, from_id, from_name)) {
          handle_open_door(chat_id);
        } else {
          send_error_unauthorized(chat_id);
        }
      } else if (msg_text == CMD_STATUS) {
        if (is_authorized(date, from_id, from_name)) {
          handle_cmd_status(chat_id);
        } else {
          send_error_unauthorized(chat_id);
        }
      } else if (msg_text == CMD_USERLS) {
        if (is_authorized(date, from_id, from_name)) {
          handle_cmd_userls(chat_id);
        } else {
          send_error_unauthorized(chat_id);
        }
      } else if (msg_text.indexOf(CMD_USERADD) >= 0) {
        if (is_authorized(date, from_id, from_name)) {
          handle_cmd_useradd(chat_id,
                             get_token(msg_text, ' ', 1),
                             get_token(msg_text, ' ', 2));
        } else {
          send_error_unauthorized(chat_id);
        }
      } else if (msg_text.indexOf(CMD_USERDEL) >= 0) {
        if (is_authorized(date, from_id, from_name)) {
          handle_cmd_userdel(chat_id, from_id,
                             get_token(msg_text, ' ', 1));
        } else {
          send_error_unauthorized(chat_id);
        }
      } else if (msg_text.indexOf(CMD_HELP) >= 0) {
        if (is_authorized(date, from_id, from_name)) {
          handle_cmd_help(chat_id);
        } else {
          handle_cmd_help_user(chat_id);
        }
      } else if (msg_text.indexOf(CMD_ID) >= 0) {
        handle_cmd_id(chat_id, from_id);
      } else if (msg_text.indexOf(CMD_LOGTAIL) >= 0) {
        if (is_authorized(date, from_id, from_name)) {
          String count = get_token(msg_text, ' ', 1);
          if (count.length() == 0) {
            count = "10";
          }
          handle_cmd_logtail(chat_id, count.toInt());
        } else {
          handle_cmd_help_user(chat_id);
        }
      }
    }
    message_count = bot->getUpdates(bot->last_message_received + 1);
  }
}

int loop_counter = 0;

void loop() {
  wait_wifi();
  delay(100);
  if (is_button_pressed()) {
    open_door();
  }
  if (loop_counter % 10 == 0) {
    handle_messages();
  }
}

//// cadoor_bot.ino ends here.
