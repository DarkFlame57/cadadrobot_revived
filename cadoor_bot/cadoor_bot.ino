
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

int Bot_mtbs = 2000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

void init_sdcard() {
  Serial.print(F("Initializing SD card..."));
  pinMode(4, OUTPUT);
  pinMode(4, HIGH);
  SPI.begin();
  while (! SD.begin(4)) {
    Serial.println(F("initialization failed!"));
    SPI.end();
    delay(1000);
    ESP.reset();
    delay(5000);
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
  delay(100);
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
//


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

char buf[20];

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

void handle_messages() {
  int message_count = bot.getUpdates(bot.last_message_received + 1);
  while (message_count) {
    Serial.print(F("Got messages: "));
    Serial.println(message_count);
    for (int i = 0; i < message_count; i++) {
      if (bot.messages[i].text == CMD_OPEN) {
        if (is_authorized(bot.messages[i].from_id)) {
          handle_open_door();
          bot.sendMessage(bot.messages[i].chat_id, "ok", "");
        } else {
          bot.sendMessage(bot.messages[i].chat_id, "not authorized", "");
        }
      } else if (bot.messages[i].text == CMD_STATUS) {
        String response = "";
        if (whitelist) {
          response += "SD card: ok";
        } else {
          response += "SD card: not connected";
        }
        bot.sendMessage(bot.messages[i].chat_id, response, "");
      }
      Serial.println(bot.messages[i].from_id);
    }
    message_count = bot.getUpdates(bot.last_message_received + 1);
  }
}

void loop() {
  delay(1000);
  handle_messages();
}
