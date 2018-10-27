/*******************************************************************
   An example of bot that echos back any messages received
*                                                                  *
   written by Giacarlo Bacchio (Gianbacchio on Github)
   adapted by Brian Lough
*******************************************************************/
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>


#include <SPI.h>
#include <SD.h>

// Bot configuration
#include "config.h"

#define DEBUG true

IPAddress ip(192, 168, 42, 15);
IPAddress gateway(192, 168, 42, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress dns1(8, 8, 8, 8);

String while_list[] = {
  "410877972"  // a_v_p
};

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

int Bot_mtbs = 1000; //mean time between scan messages
long Bot_lasttime;   //last time messages' scan has been done

void init_sdcard() {
  Serial.print("Initializing SD card...");

  if (!SD.begin(4)) {
    Serial.println("initialization failed!");
    while (1) {
      delay(100);
    }
  }
  Serial.println("initialization done.");

}

File myFile;

void setup() {
  Serial.begin(115200);
  delay(500);

  init_sdcard();
  myFile = SD.open("white_list.txt");

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);

  // Attempt to connect to Wifi network:
  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);
  WiFi.config(ip, gateway, subnet, dns1);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

enum STATE {
  CHECK_ID,
  GET_ID
};

bool is_authorized(String id) {
//  while (myFile.available()) {
//    String line = myFile.readStringUntil('\n');
//    //Serial.println(line);
//    if (line == id) {
//      return true;
//    }
//  }
//  return false;
  return true;
}

void handle_open_door() {
  if (DEBUG)
    Serial.println("[debug] handle_open_door");
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
}

void handle_messages() {
  if (DEBUG)
    Serial.println("[debug] handle_open_door");
  int message_count = bot.getUpdates(bot.last_message_received + 1);
  while (message_count) {
    Serial.print("Got messages: ");
    Serial.println(message_count);
    for (int i = 0; i < message_count; i++) {
      if (bot.messages[i].text == "/open") {
        Serial.print(bot.messages[i].from_id);
        Serial.println(": /open");
        if (is_authorized(bot.messages[i].from_id)) {
          handle_open_door();
          bot.sendMessage(bot.messages[i].chat_id, "ok", "");
        } else {
          bot.sendMessage(bot.messages[i].chat_id, "not authorized", "");
        }
      }
      Serial.println(bot.messages[i].from_id);
    }
    message_count = bot.getUpdates(bot.last_message_received + 1);
  }
}

void loop() {
  if (millis() > Bot_lasttime + Bot_mtbs) {
    handle_messages();
    Bot_lasttime = millis();
  }
}
