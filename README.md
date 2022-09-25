
#          ESPHome Custom Component Modbus sniffer for Delta Solvia Inverter 3.0 EU G4 TR installed with Solivia Gateway M1 D2.

Remember to copy the Custom Component solivia.h file to the ESPHome folder in
Home Assistant !!

This config doesn't send any commands to the inverter !!
Instead it relies on the gateways constant request for data (appx. 1 package pr. second).

If you don't have a gateway, the package request can instead easily be send from
ESPHome using uart.write and eg. triggered via the ESPHome Time component.

Example:
```yaml
time:
  - platform: homeassistant
    id: esptime
  - platform: sntp
    on_time:
       // Request package every 10 seconds from inverter at slave address: 0x01
      - seconds: /10
        then:
          - uart.write: [0x02, 0x05, 0x01, 0x02, 0x60, 0x01, 0x85, 0xFC, 0x03]
```

NOTE !!
My inverter, unlike most examples found on the net, returns a 255 bytes
response. Most common inverter response length is 150 bytes (0x96) or 157 (0x9D).
So almost all commands/registers do not match other examples on the net.
Package structure is also somewhat different.

Below the actual ESPHome config, you'll find a compiled list of all registers
I've identified. Those marked with a question mark are not confirmed,
but an 'educated guess' only.

Tested on both ESP8266 with software uart and ESP32 with hardware uart.
I experience no issues in production with the ESP8266 software uart.
Not even with debug uart logging active.

My config:
Inverter part no.: EOE46010287
Single string input PV1: 3500W
Single phase output: L1
Slave address: 0x01
Baud rate: 19200
Install country: Denmark

```yaml
Gateway request: 02:05:01:02:60:01:85:FC:03
Inverter response: 02:06:01:FF:60:01 + 255 data bytes (incl. CRC bytes) + ETX byte
```

Actually the response doesn't match the protocol, as the CRC bytes and trailing
ETX byte should be excluded from data length identifier (0xff)
Here, strangely enough, the CRC is included, but not the ETX ?

A few of the 'public known' commands have been tested. Most unfortunately did fail.
Haven't really spend much time on testing further commands, as all the data
i need is in gateway package.
But commmand for eg. inverters serial no. is working ok on my inverter.
- uart.write: [0x02, 0x05, 0x01, 0x02, 0x00, 0x01, 0xAD, 0xFC, 0x03] will response
 correctly with serial no.

Complete 'live' package example - captured [2022-08-08 10:32:54 GMT+1]:

```yaml
Request:  02:05:01:02:60:01:85:FC:03
Response: 02:06:01:FF:60:01
Package data:
0x00   45:4F:45:34:36:30:31:30:32:38:37:31:31:33:32:38
0x10   37:30:38:31:33:30:31:30:30:33:33:39:38:31:33:30
0x20   31:30:38:01:02:1A:00:00:00:00:23:34:00:00:00:00
0x30   23:34:00:00:00:00:00:00:00:00:00:00:23:34:00:00
0x40   00:00:00:00:00:00:00:00:01:00:03:96:01:9A:00:16
0x50   00:00:00:00:00:00:00:00:00:00:00:00:00:23:00:EC
0x60   13:88:03:64:FF:4E:00:00:00:00:00:00:00:00:00:00
0x70   00:00:00:00:00:00:00:00:00:00:08:98:07:D0:00:33
0x80   00:33:00:00:00:00:02:14:3E:3E:00:26:48:A6:00:00
0x90   00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
0xA0   00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
0xB0   00:00:00:00:08:F7:00:00:01:16:00:00:00:00:00:00
0xC0   00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
0xD0   00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
0xE0   00:00:00:00:00:00:00:00:00:00:00:00:00:00:00:00
0xF0   00:00:00:00:00:00:00:00:00:00:00:00:00:34:A4:03

Register address:

0x00 - 0x0A:	Part no.                  EOE46010287           45:4F:45:34:36:30:31:30:32:38:37
0x0B - 0x1C:	Serial no.                113287081301003398    31:31:33:32:38:37:30:38:31:33:30:31:30:30:33:33:39:38
0x1D - 0x22:  Install date (YYMMDD) ?   130108                31:33:30:31:30:38
0x23 - 0x25:  FW rev. system            1.02.26               01:02:1A
0x29 - 0x2B:  FW Rev. DC, AC or RTS ?   0.35.52               00:23:34
0x2F - 0x31:  FW Rev. DC, AC or RTS ?   0.35.52               00:23:34
0x3B - 0x3D:  FW Rev. DC, AC or RTS ?   0.35.52               00:23:34
0x48 - 0x48:  Inverter slave address ?  1                     01
0x4A - 0x4B:  DC Power PV1 ?            918                   03:96
0x4C - 0x4D:  DC voltage PV1            410                   01:9A
0x4E - 0x4F:  DC current PV1 (2,2 A)    22                    00:16
0x5C - 0x5D:  AC current L1 (3,5 A)     35                    00:23
0x5E - 0x5F:  AC voltage L1             236                   00:EC
0x60 - 0x61:  AC frequency (50,00 Hz)   5000                  13:88
0x62 - 0x63:  AC Power L1               868                   03:64
0x64 - 0x64:  Unknown ?                 255                   FF
0x65 - 0x65:  Unknown ?                 78                    4E
0x7A - 0x7B:  ISO+ resistance kΩ        2200                  08:98
0x7C - 0x7D:  ISO- resistance kΩ        2000                  07:D0
0x7E - 0x7F:  Heat sink AC (50°C) ?     50                    00:33
0x80 - 0x81:  Heat sink DC (50°C) ?     50                    00:33
0x86 - 0x89   Total yield(34881,086kWh) 34881086              02:14:3E:3E
0x8A - 0x8C:  Uptime total in minutes   2508966               00:26:48:A6
0x91 - 0x91:  Inverter status ?         0                     00
0xB4 - 0xB5:  Daily power yield Wh      2295                  08:F7
0xB8 - 0xB9:  Uptime today in minutes   278                   01:16```

Observations on the unknown registers:

0x64 - 0x64: Unknown register ?
Only 0x00, 0xff and some rare 0xfe value have been observed. 0x00 when there's no production. Otherwise 0xFF

0x65 - 0x65: Unknown register ?
Fluctuates throughout the day. DC injector value mA ??

0x7E - 0x7F: AC Heat sink register ?
0x80 - 0x81: DC Heat sink register ?
The two heat sink registers has during a three day test period been completely identical.
So there's most likely only one physical sensor in my inverter.
When inverter is is production mode, values have been in the range from 36 - 62°C throughout the day (high summer period)

0x91 - 0x91: Device status register ?
When inverter is active value is 0. When inactive value is 4
On device init in the morning value shifts rapidly from 4 to 6, from 6 to 8, from 8 back to 2 and finally to 0 (inverter active)
Sometimes values above 4 are also observed during inverter shutdown in the evening.
