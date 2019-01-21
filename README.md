# cadadrobot
Hackerspace door Telegram bot.

## Dependencies
- [ESP8266](https://github.com/esp8266/Arduino) 2.4.2
- [UniversalTelegramBot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot) 1.1.0
- [ArduinoJson](https://arduinojson.org/) 5.13.3

## Building and installation 
Select "Generic ESP8266 module" board in the "Tools" menu and build
the sources.

## Usage

### Configuration

The bot configuration is stored in `CONF.TXT` file on SD card.  The
format of the configuration file is the following:

```
ssid:<the-ssid-of-your-wifi-ap>
password:<your-wifi-password>
token:<your-telegram-token>
```

### Whitelist

The list of the people who authorized to manage bot and open the
hackerspace door is stored in `WL.TXT` file on the SD card.  The
format is following:

```
<telegram-user-id-1>:<nickname-or-real-name>
<telegram-user-id-2>:<nickname-or-real-name>
...
<telegram-user-id-n>:<nickname-or-real-name>
```

The 1st record in the file describes the superadmin.  The bot protects
the this line from the file from deletion so it cannot be removed by
anyone using the Telegram commands.

### Logging

The bot logs every event of authorized access to its functions to the
file named `LOG.TXT` on the SD card.

