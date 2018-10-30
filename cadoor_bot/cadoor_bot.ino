
/*******************************************************************
   An example of bot that echos back any messages received
*                                                                  *
   written by Giacarlo Bacchio (Gianbacchio on Github)
   adapted by Brian Lough
*******************************************************************/
#include <SPI.h>
#include <SD.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

// Bot configuration
#include "config.h"

#define DEBUG true

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
File whitelist;

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

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    delay(500);
  }
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(1000);
  init_sdcard();
  delay(1000);
  whitelist = SD.open("WL.TXT", FILE_WRITE);
  whitelist.close();

  whitelist = SD.open("WL.TXT");
  if (! whitelist) {
    Serial.println(F("[error] could not open file"));
    return;
  }
  is_authorized("123");
  // Attempt to connect to Wifi network:
  Serial.print(F("Connecting Wifi: "));
  Serial.println(ssid);
  IPAddress ip(192, 168, 42, 15);
  IPAddress gateway(192, 168, 42, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns1(8, 8, 8, 8);
  WiFi.config(ip, gateway, subnet, dns1);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println(F("\nWiFi connected"));
  Serial.print(F("IP address: "));
  Serial.println(WiFi.localIP());

  configTime(0 * 3600, 0, "pool.ntp.org", "time.nist.gov");

}

extern "C" {
#include "user_interface.h"
}

bool is_authorized(String id) {
  bool result = false;
  if (DEBUG)
    Serial.println(F("[debug] is_authorized"));
  whitelist.seek(0);
  while (whitelist.available()) {
    String line = whitelist.readStringUntil('\n');
    if (line.length() == 0)
      result = false;
    if (line.compareTo(id)) {
      result = true;
    }
    delay(100);
  }

  return result;
}

void handle_open_door() {
  if (DEBUG)
    Serial.println("[debug] handle_open_door");
}

const char* CMD_OPEN = "/open";
const char* CMD_STATUS = "/status";
const char* CMD_USERADD = "/useradd";
const char* CMD_USERLS  = "/userls";

void handle_cmd_status(String chat_id) {
  String response = "";
  response += String("Free RAM: ") + system_get_free_heap_size() + "\n";
  if (whitelist) {
    response += "SD card:\tok";
  } else {
    response += "SD card:\tnot connected";
  }
  bot.sendMessage(chat_id, response, "");
}

void handle_cmd_userls(String chat_id) {
  String response = "";
  int counter = 0;
  whitelist.seek(0);
  for (int idx = 0; whitelist.available(); ++idx) {
    String line = whitelist.readStringUntil('\n');
    response += String("") + idx + ": " + line + "\n";
  }
  bot.sendMessage(chat_id, response, "");
}

void send_error_unauthorized(String chat_id) {
  bot.sendMessage(chat_id, "not authorized", "");
}

void send_ok(String chat_id) {
  bot.sendMessage(chat_id, "ok", "");
}

void handle_messages() {
  int message_count = bot.getUpdates(bot.last_message_received + 1);
  while (message_count) {
    Serial.print(F("Got messages: "));
    Serial.println(message_count);
    for (int i = 0; i < message_count; i++) {
      String chat_id = bot.messages[i].chat_id;
      String from_id = bot.messages[i].from_id;
      String msg_text = bot.messages[i].text;
      if (msg_text == CMD_OPEN) {
        if (is_authorized(from_id)) {
          handle_open_door();
          send_ok(chat_id);
        } else {
          send_error_unauthorized(chat_id);
        }
      } else if (msg_text == CMD_STATUS) {
        if (is_authorized(from_id)) {
          handle_cmd_status(chat_id);
        } else {
          send_error_unauthorized(chat_id);
        }
      } else if (msg_text == CMD_USERLS) {
        if (is_authorized(from_id)) {
          handle_cmd_userls(chat_id);
        } else {
          send_error_unauthorized(chat_id);
        }
      }
    }
    message_count = bot.getUpdates(bot.last_message_received + 1);
  }
}

void loop() {
  delay(1000);
  handle_messages();
}
