# cadadrobot
Hackerspace door Telegram bot.

## Dependencies
- [ESP8266](https://github.com/esp8266/Arduino) 2.4.2
- [UniversalTelegramBot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot) 1.1.0
- [ArduinoJson](https://arduinojson.org/) 5.13.3

## Configuration

The bot configuration is stored in `CONF.TXT` file on SD card.  The
format of the configuration file is the following:

```
ssid:<the-ssid-of-your-wifi-ap>
password:<your-wifi-password>
token:<your-telegram-token>
```

## Building and installation 
Select "Generic ESP8266 module" board in the "Tools" menu and build
the sources.