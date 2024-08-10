# BlueZ-DBus

This program is an attempt to learn recieving Bluetooth LE Advertisment recieving using the libdbus-1-dev library 

https://dbus.freedesktop.org/doc/api/html/index.html

I started with a code example I found at https://forums.raspberrypi.com/viewtopic.php?t=353889&sid=2b10dcb0eca390f135ee8c64da253a9c and heavily modified to my own style.

# The program has fulfilled it's original purpose.
1. Connect to DBus
2. Enumerate Bluetooth Adapters
3. Power on a Bluetooth Adapter
4. Enable Scanning for Bluetooth LE devices (In DBus Parlance, Discovery)
5. Enable Messages from DBus to get the equivelent of Bluetooth Advertising Messages
6. Decode Bluetooth Messages to be able to recognize temperature data stored in Manufacturer Data

While testing the program I've learned a few interesting things. 

## Running HCI Bluetooth Programs Are Not apparent

My [old project](https://github.com/wcbonner/GoveeBTTempLogger) was developed using the now deprecated hci interface to the bluetooth hardware.
I've got it running on a machine with a pair of bluetooth adapters. 
Running the user level commands supplied with modern linux to examine the 
bluetooth adapters doesn't make any distinction to indicate that one of 
the hardware devices may be currently in use.

```
wim@WimPi4-Sola:~ $ bluetoothctl
Agent registered
[CHG] Controller DC:A6:32:D5:3F:2E Pairable: yes
[CHG] Controller F4:4E:FC:A0:F7:DA Pairable: yes
[bluetooth]# show DC:A6:32:D5:3F:2E
Controller DC:A6:32:D5:3F:2E (public)
        Name: WimPi4-Sola #1
        Alias: WimPi4-Sola #1
        Class: 0x00000000
        Powered: yes
        Discoverable: no
        DiscoverableTimeout: 0x000000b4
        Pairable: yes
        UUID: Generic Attribute Profile (00001801-0000-1000-8000-00805f9b34fb)
        UUID: Generic Access Profile    (00001800-0000-1000-8000-00805f9b34fb)
        UUID: PnP Information           (00001200-0000-1000-8000-00805f9b34fb)
        UUID: A/V Remote Control Target (0000110c-0000-1000-8000-00805f9b34fb)
        UUID: A/V Remote Control        (0000110e-0000-1000-8000-00805f9b34fb)
        UUID: Device Information        (0000180a-0000-1000-8000-00805f9b34fb)
        Modalias: usb:v1D6Bp0246d0542
        Discovering: no
        Roles: central
        Roles: peripheral
Advertising Features:
        ActiveInstances: 0x00 (0)
        SupportedInstances: 0x05 (5)
        SupportedIncludes: tx-power
        SupportedIncludes: appearance
        SupportedIncludes: local-name
[bluetooth]# show F4:4E:FC:A0:F7:DA
Controller F4:4E:FC:A0:F7:DA (public)
        Name: WimPi4-Sola #2
        Alias: WimPi4-Sola #2
        Class: 0x00000000
        Powered: yes
        Discoverable: no
        DiscoverableTimeout: 0x000000b4
        Pairable: yes
        UUID: Generic Attribute Profile (00001801-0000-1000-8000-00805f9b34fb)
        UUID: Generic Access Profile    (00001800-0000-1000-8000-00805f9b34fb)
        UUID: PnP Information           (00001200-0000-1000-8000-00805f9b34fb)
        UUID: A/V Remote Control Target (0000110c-0000-1000-8000-00805f9b34fb)
        UUID: A/V Remote Control        (0000110e-0000-1000-8000-00805f9b34fb)
        UUID: Device Information        (0000180a-0000-1000-8000-00805f9b34fb)
        Modalias: usb:v1D6Bp0246d0542
        Discovering: no
        Roles: central
        Roles: peripheral
Advertising Features:
        ActiveInstances: 0x00 (0)
        SupportedInstances: 0x05 (5)
        SupportedIncludes: tx-power
        SupportedIncludes: appearance
        SupportedIncludes: local-name
[bluetooth]# exit
```

## Bluetooth LE Advertisment Messages are absorbed by BlueZ

I'm interested in the data that's being broadcast as a Bluetooth LE advertisment message. 
I've not found a way to explicitly get advertising messages.
BlueZ maintains a list of devices and their properties. 
I can tell BlueZ to start scanning for devices. I can tell it to filter by various items. 
https://www.mankier.com/5/org.bluez.Adapter#Interface-void_SetDiscoveryFilter(dict_filter
I can't seem to specify the hardware timing that I can via the HCI interface.

The DBus library or platform returns man items using human readable text as keys which may be better used in a langauge other than C/C++.


I ran btmon on the raspberry and got the following results. 
```
wim@WimPi5:~ $ sudo btmon --date
Bluetooth monitor ver 5.66
= Note: Linux version 6.6.31+rpt-rpi-2712 (aarch64)                                   2024-08-09 09:39:34.966144
= Note: Bluetooth subsystem version 2.22                                              2024-08-09 09:39:34.966146
= New Index: 2C:CF:67:0B:78:71 (Primary,UART,hci0)                             [hci0] 2024-08-09 09:39:34.966147
= Open Index: 2C:CF:67:0B:78:71                                                [hci0] 2024-08-09 09:39:34.966147
= Index Info: 2C:CF:67:0B:78:71 (Cypress Semiconductor)                        [hci0] 2024-08-09 09:39:34.966148
@ MGMT Open: bluetoothd (privileged) version 1.22                            {0x0001} 2024-08-09 09:39:34.966149

@ MGMT Command: Start Service Discovery (0x003a) plen 4               {0x0001} [hci0] 2024-08-09 09:41:39.255301
        Address type: 0x06
          LE Public
          LE Random
        RSSI: invalid (0x7f)
        UUIDs: 0
< HCI Command: LE Set Random Address (0x08|0x0005) plen 6                   #1 [hci0] 2024-08-09 09:41:39.255376
        Address: 1D:43:5A:0A:81:1C (Non-Resolvable)
> HCI Event: Command Complete (0x0e) plen 4                                 #2 [hci0] 2024-08-09 09:41:39.255666
      LE Set Random Address (0x08|0x0005) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Parameters (0x08|0x000b) plen 7                  #3 [hci0] 2024-08-09 09:41:39.255684
        Type: Active (0x01)
        Interval: 11.250 msec (0x0012)
        Window: 11.250 msec (0x0012)
        Own address type: Random (0x01)
        Filter policy: Accept all advertisement (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                 #4 [hci0] 2024-08-09 09:41:39.255971
      LE Set Scan Parameters (0x08|0x000b) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                      #5 [hci0] 2024-08-09 09:41:39.255980
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                 #6 [hci0] 2024-08-09 09:41:39.256282
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
@ MGMT Event: Command Complete (0x0001) plen 4                        {0x0001} [hci0] 2024-08-09 09:41:39.256291
      Start Service Discovery (0x003a) plen 1
        Status: Success (0x00)
        Address type: 0x06
          LE Public
          LE Random
@ MGMT Event: Discovering (0x0013) plen 2                             {0x0001} [hci0] 2024-08-09 09:41:39.256294
        Address type: 0x06
          LE Public
          LE Random
        Discovery: Enabled (0x01)
> HCI Event: LE Meta Event (0x3e) plen 43                                   #7 [hci0] 2024-08-09 09:41:39.259011
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -62 dBm (0xc2)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:39.259022
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -62 dBm (0xc2)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 43                                   #8 [hci0] 2024-08-09 09:41:39.330701
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:39.330720
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                      #9 [hci0] 2024-08-09 09:41:39.461131
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                #10 [hci0] 2024-08-09 09:41:39.462890
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #11 [hci0] 2024-08-09 09:41:39.462904
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                #12 [hci0] 2024-08-09 09:41:39.463240
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 31                                  #13 [hci0] 2024-08-09 09:41:39.472488
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
        RSSI: -61 dBm (0xc3)
> HCI Event: LE Meta Event (0x3e) plen 43                                  #14 [hci0] 2024-08-09 09:41:39.474485
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 33                           {0x0001} [hci0] 2024-08-09 09:41:39.474491
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -61 dBm (0xc3)
        Flags: 0x00000000
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:39.474494
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 43                                  #15 [hci0] 2024-08-09 09:41:39.541217
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -53 dBm (0xcb)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:39.541234
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -53 dBm (0xcb)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
> HCI Event: LE Meta Event (0x3e) plen 12                                  #16 [hci0] 2024-08-09 09:41:39.596716
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 0
        RSSI: -63 dBm (0xc1)
@ MGMT Event: Device Found (0x0012) plen 14                           {0x0001} [hci0] 2024-08-09 09:41:39.596737
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -63 dBm (0xc1)
        Flags: 0x00000020
          Unknown device flag (0x00000020)
        Data length: 0
> HCI Event: LE Meta Event (0x3e) plen 41                                  #17 [hci0] 2024-08-09 09:41:39.633750
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        RSSI: -47 dBm (0xd1)
> HCI Event: LE Meta Event (0x3e) plen 34                                  #18 [hci0] 2024-08-09 09:41:39.634537
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -48 dBm (0xd0)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-09 09:41:39.634544
        LE Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        RSSI: -48 dBm (0xd0)
        Flags: 0x00000000
        Data length: 51
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        Name (complete): Cerbo GX HQ2312AG767
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #19 [hci0] 2024-08-09 09:41:39.677133
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                #20 [hci0] 2024-08-09 09:41:39.679148
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #21 [hci0] 2024-08-09 09:41:39.679163
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                #22 [hci0] 2024-08-09 09:41:39.679571
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 43                                  #23 [hci0] 2024-08-09 09:41:39.749502
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:39.749513
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
> HCI Event: LE Meta Event (0x3e) plen 43                                  #24 [hci0] 2024-08-09 09:41:39.780650
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -55 dBm (0xc9)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:39.780667
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -55 dBm (0xc9)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 31                                  #25 [hci0] 2024-08-09 09:41:39.787364
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
        RSSI: -63 dBm (0xc1)
> HCI Event: LE Meta Event (0x3e) plen 12                                  #26 [hci0] 2024-08-09 09:41:39.787970
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 0
        RSSI: -64 dBm (0xc0)
@ MGMT Event: Device Found (0x0012) plen 33                           {0x0001} [hci0] 2024-08-09 09:41:39.787976
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -64 dBm (0xc0)
        Flags: 0x00000000
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
> HCI Event: LE Meta Event (0x3e) plen 41                                  #27 [hci0] 2024-08-09 09:41:39.794999
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        RSSI: -48 dBm (0xd0)
> HCI Event: LE Meta Event (0x3e) plen 34                                  #28 [hci0] 2024-08-09 09:41:39.795787
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -49 dBm (0xcf)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-09 09:41:39.795793
        LE Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        RSSI: -49 dBm (0xcf)
        Flags: 0x00000000
        Data length: 51
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        Name (complete): Cerbo GX HQ2312AG767
> HCI Event: LE Meta Event (0x3e) plen 37                                  #29 [hci0] 2024-08-09 09:41:39.915525
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: A4:C1:38:05:C7:A1 (Telink Semiconductor (Taipei) Co. Ltd.)
        Data length: 25
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Name (complete): Govee_H5074_C7A1
        RSSI: -64 dBm (0xc0)
> HCI Event: LE Meta Event (0x3e) plen 41                                  #30 [hci0] 2024-08-09 09:41:39.947543
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: E3:5E:CC:21:5C:0F (OUI E3-5E-CC)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_5C0F
        RSSI: -61 dBm (0xc3)
@ MGMT Event: Device Found (0x0012) plen 39                           {0x0001} [hci0] 2024-08-09 09:41:39.947558
        LE Address: A4:C1:38:05:C7:A1 (Telink Semiconductor (Taipei) Co. Ltd.)
        RSSI: -64 dBm (0xc0)
        Flags: 0x00000000
        Data length: 25
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Name (complete): Govee_H5074_C7A1
> HCI Event: LE Meta Event (0x3e) plen 39                                  #31 [hci0] 2024-08-09 09:41:39.948234
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: E3:5E:CC:21:5C:0F (OUI E3-5E-CC)
        Data length: 27
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 29776.23567
          TX power: -62 dB
        RSSI: -62 dBm (0xc2)
@ MGMT Event: Device Found (0x0012) plen 70                           {0x0001} [hci0] 2024-08-09 09:41:39.948242
        LE Address: E3:5E:CC:21:5C:0F (OUI E3-5E-CC)
        RSSI: -62 dBm (0xc2)
        Flags: 0x00000000
        Data length: 56
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_5C0F
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 29776.23567
          TX power: -62 dB
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #32 [hci0] 2024-08-09 09:41:39.953172
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                #33 [hci0] 2024-08-09 09:41:39.955429
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #34 [hci0] 2024-08-09 09:41:39.955478
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                #35 [hci0] 2024-08-09 09:41:39.955793
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 41                                  #36 [hci0] 2024-08-09 09:41:39.980960
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: E3:60:59:23:14:7D (OUI E3-60-59)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_147D
        RSSI: -66 dBm (0xbe)
> HCI Event: LE Meta Event (0x3e) plen 39                                  #37 [hci0] 2024-08-09 09:41:39.981871
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: E3:60:59:23:14:7D (OUI E3-60-59)
        Data length: 27
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 29776.5245
          TX power: -62 dB
        RSSI: -68 dBm (0xbc)
@ MGMT Event: Device Found (0x0012) plen 70                           {0x0001} [hci0] 2024-08-09 09:41:39.981879
        LE Address: E3:60:59:23:14:7D (OUI E3-60-59)
        RSSI: -68 dBm (0xbc)
        Flags: 0x00000000
        Data length: 56
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_147D
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 29776.5245
          TX power: -62 dB
> HCI Event: LE Meta Event (0x3e) plen 43                                  #38 [hci0] 2024-08-09 09:41:40.061967
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:40.061984
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
> HCI Event: LE Meta Event (0x3e) plen 43                                  #39 [hci0] 2024-08-09 09:41:40.083731
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -55 dBm (0xc9)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:40.083749
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -55 dBm (0xc9)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 41                                  #40 [hci0] 2024-08-09 09:41:40.111883
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        RSSI: -42 dBm (0xd6)
> HCI Event: LE Meta Event (0x3e) plen 34                                  #41 [hci0] 2024-08-09 09:41:40.112717
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -42 dBm (0xd6)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-09 09:41:40.112723
        LE Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        RSSI: -42 dBm (0xd6)
        Flags: 0x00000000
        Data length: 51
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        Name (complete): Cerbo GX HQ2312AG767
> HCI Event: LE Meta Event (0x3e) plen 41                                  #42 [hci0] 2024-08-09 09:41:40.114342
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: E3:8E:C8:C1:98:9A (OUI E3-8E-C8)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_989A
        RSSI: -49 dBm (0xcf)
> HCI Event: LE Meta Event (0x3e) plen 39                                  #43 [hci0] 2024-08-09 09:41:40.115244
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: E3:8E:C8:C1:98:9A (OUI E3-8E-C8)
        Data length: 27
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 29776.39066
          TX power: -62 dB
        RSSI: -49 dBm (0xcf)
@ MGMT Event: Device Found (0x0012) plen 70                           {0x0001} [hci0] 2024-08-09 09:41:40.115251
        LE Address: E3:8E:C8:C1:98:9A (OUI E3-8E-C8)
        RSSI: -49 dBm (0xcf)
        Flags: 0x00000000
        Data length: 56
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_989A
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 29776.39066
          TX power: -62 dB
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #44 [hci0] 2024-08-09 09:41:40.185133
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                #45 [hci0] 2024-08-09 09:41:40.186652
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #46 [hci0] 2024-08-09 09:41:40.186667
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                #47 [hci0] 2024-08-09 09:41:40.187001
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 43                                  #48 [hci0] 2024-08-09 09:41:40.269979
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:40.269994
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
> HCI Event: LE Meta Event (0x3e) plen 43                                  #49 [hci0] 2024-08-09 09:41:40.290847
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -65 dBm (0xbf)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:40.290862
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -65 dBm (0xbf)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 37                                  #50 [hci0] 2024-08-09 09:41:40.312466
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: A4:C1:38:D5:A3:3B (Telink Semiconductor (Taipei) Co. Ltd.)
        Data length: 25
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Name (complete): Govee_H5074_A33B
        RSSI: -55 dBm (0xc9)
> HCI Event: LE Meta Event (0x3e) plen 39                                  #51 [hci0] 2024-08-09 09:41:40.313318
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: A4:C1:38:D5:A3:3B (Telink Semiconductor (Taipei) Co. Ltd.)
        Data length: 27
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: -62 dB
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 66                           {0x0001} [hci0] 2024-08-09 09:41:40.313325
        LE Address: A4:C1:38:D5:A3:3B (Telink Semiconductor (Taipei) Co. Ltd.)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000000
        Data length: 52
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Name (complete): Govee_H5074_A33B
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: -62 dB
> HCI Event: LE Meta Event (0x3e) plen 41                                  #52 [hci0] 2024-08-09 09:41:40.422505
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        RSSI: -42 dBm (0xd6)
> HCI Event: LE Meta Event (0x3e) plen 34                                  #53 [hci0] 2024-08-09 09:41:40.423242
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -42 dBm (0xd6)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-09 09:41:40.423248
        LE Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        RSSI: -42 dBm (0xd6)
        Flags: 0x00000000
        Data length: 51
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        Name (complete): Cerbo GX HQ2312AG767
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #54 [hci0] 2024-08-09 09:41:40.473133
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                #55 [hci0] 2024-08-09 09:41:40.475405
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #56 [hci0] 2024-08-09 09:41:40.475421
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                #57 [hci0] 2024-08-09 09:41:40.475755
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 43                                  #58 [hci0] 2024-08-09 09:41:40.498194
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -55 dBm (0xc9)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:40.498203
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -55 dBm (0xc9)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 31                                  #59 [hci0] 2024-08-09 09:41:40.511754
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
        RSSI: -62 dBm (0xc2)
> HCI Event: LE Meta Event (0x3e) plen 12                                  #60 [hci0] 2024-08-09 09:41:40.512256
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 0
        RSSI: -62 dBm (0xc2)
@ MGMT Event: Device Found (0x0012) plen 33                           {0x0001} [hci0] 2024-08-09 09:41:40.512262
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -62 dBm (0xc2)
        Flags: 0x00000000
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
> HCI Event: LE Meta Event (0x3e) plen 43                                  #61 [hci0] 2024-08-09 09:41:40.544954
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: C2:35:33:30:25:50 (Static)
        Data length: 31
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): GVH5100_2550
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Company: Nokia Mobile Phones (1)
          Data: 01010308342c
        RSSI: -46 dBm (0xd2)
> HCI Event: LE Meta Event (0x3e) plen 39                                  #62 [hci0] 2024-08-09 09:41:40.545793
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: C2:35:33:30:25:50 (Static)
        Data length: 27
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: 12 dB
        RSSI: -44 dBm (0xd4)
@ MGMT Event: Device Found (0x0012) plen 72                           {0x0001} [hci0] 2024-08-09 09:41:40.545799
        LE Address: C2:35:33:30:25:50 (Static)
        RSSI: -44 dBm (0xd4)
        Flags: 0x00000000
        Data length: 58
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): GVH5100_2550
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Company: Nokia Mobile Phones (1)
          Data: 01010308342c
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: 12 dB
> HCI Event: LE Meta Event (0x3e) plen 43                                  #63 [hci0] 2024-08-09 09:41:40.580262
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:40.580278
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #64 [hci0] 2024-08-09 09:41:40.701131
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                #65 [hci0] 2024-08-09 09:41:40.702909
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #66 [hci0] 2024-08-09 09:41:40.702925
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                #67 [hci0] 2024-08-09 09:41:40.703259
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 43                                  #68 [hci0] 2024-08-09 09:41:40.708353
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -64 dBm (0xc0)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:40.708360
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -64 dBm (0xc0)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 31                                  #69 [hci0] 2024-08-09 09:41:40.713024
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
        RSSI: -63 dBm (0xc1)
> HCI Event: LE Meta Event (0x3e) plen 12                                  #70 [hci0] 2024-08-09 09:41:40.713390
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 0
        RSSI: -65 dBm (0xbf)
@ MGMT Event: Device Found (0x0012) plen 33                           {0x0001} [hci0] 2024-08-09 09:41:40.713397
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -65 dBm (0xbf)
        Flags: 0x00000000
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
> HCI Event: LE Meta Event (0x3e) plen 43                                  #71 [hci0] 2024-08-09 09:41:40.735798
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: D0:35:33:33:44:03 (Static)
        Data length: 31
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): GVH5105_4403
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Company: Nokia Mobile Phones (1)
          Data: 010103236964
        RSSI: -53 dBm (0xcb)
> HCI Event: LE Meta Event (0x3e) plen 39                                  #72 [hci0] 2024-08-09 09:41:40.736546
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: D0:35:33:33:44:03 (Static)
        Data length: 27
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: 12 dB
        RSSI: -53 dBm (0xcb)
@ MGMT Event: Device Found (0x0012) plen 72                           {0x0001} [hci0] 2024-08-09 09:41:40.736553
        LE Address: D0:35:33:33:44:03 (Static)
        RSSI: -53 dBm (0xcb)
        Flags: 0x00000000
        Data length: 58
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): GVH5105_4403
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Company: Nokia Mobile Phones (1)
          Data: 010103236964
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: 12 dB
> HCI Event: LE Meta Event (0x3e) plen 41                                  #73 [hci0] 2024-08-09 09:41:40.743147
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        RSSI: -48 dBm (0xd0)
> HCI Event: LE Meta Event (0x3e) plen 34                                  #74 [hci0] 2024-08-09 09:41:40.743974
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -48 dBm (0xd0)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-09 09:41:40.743984
        LE Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        RSSI: -48 dBm (0xd0)
        Flags: 0x00000000
        Data length: 51
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        Name (complete): Cerbo GX HQ2312AG767
> HCI Event: LE Meta Event (0x3e) plen 43                                  #75 [hci0] 2024-08-09 09:41:40.762639
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: A4:C1:38:0D:3B:10 (Telink Semiconductor (Taipei) Co. Ltd.)
        Data length: 31
        Name (complete): GVH5177_3B10
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Flags: 0x05
          LE Limited Discoverable Mode
          BR/EDR Not Supported
        Company: Nokia Mobile Phones (1)
          Data: 010102f89b54
        RSSI: -63 dBm (0xc1)
> HCI Event: LE Meta Event (0x3e) plen 39                                  #76 [hci0] 2024-08-09 09:41:40.763325
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: A4:C1:38:0D:3B:10 (Telink Semiconductor (Taipei) Co. Ltd.)
        Data length: 27
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30545.65522
          TX power: -62 dB
        RSSI: -64 dBm (0xc0)
@ MGMT Event: Device Found (0x0012) plen 72                           {0x0001} [hci0] 2024-08-09 09:41:40.763334
        LE Address: A4:C1:38:0D:3B:10 (Telink Semiconductor (Taipei) Co. Ltd.)
        RSSI: -64 dBm (0xc0)
        Flags: 0x00000000
        Data length: 58
        Name (complete): GVH5177_3B10
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Flags: 0x05
          LE Limited Discoverable Mode
          BR/EDR Not Supported
        Company: Nokia Mobile Phones (1)
          Data: 010102f89b54
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30545.65522
          TX power: -62 dB
> HCI Event: LE Meta Event (0x3e) plen 43                                  #77 [hci0] 2024-08-09 09:41:40.786257
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:40.786278
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #78 [hci0] 2024-08-09 09:41:40.909144
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                #79 [hci0] 2024-08-09 09:41:40.910415
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #80 [hci0] 2024-08-09 09:41:40.910434
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                #81 [hci0] 2024-08-09 09:41:40.910763
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 43                                  #82 [hci0] 2024-08-09 09:41:40.922021
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -55 dBm (0xc9)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:40.922034
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -55 dBm (0xc9)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 20                                  #83 [hci0] 2024-08-09 09:41:40.936093
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: C4:A0:4D:54:0E:11 (Static)
        Data length: 8
        Company: Apple, Inc. (76)
          Type: Unknown (18)
          Data: 2402
        RSSI: -63 dBm (0xc1)
> HCI Event: LE Meta Event (0x3e) plen 12                                  #84 [hci0] 2024-08-09 09:41:40.936737
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: C4:A0:4D:54:0E:11 (Static)
        Data length: 0
        RSSI: -64 dBm (0xc0)
@ MGMT Event: Device Found (0x0012) plen 22                           {0x0001} [hci0] 2024-08-09 09:41:40.936745
        LE Address: C4:A0:4D:54:0E:11 (Static)
        RSSI: -64 dBm (0xc0)
        Flags: 0x00000000
        Data length: 8
        Company: Apple, Inc. (76)
          Type: Unknown (18)
          Data: 2402
> HCI Event: LE Meta Event (0x3e) plen 41                                  #85 [hci0] 2024-08-09 09:41:40.944466
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: E3:5E:CC:21:5C:0F (OUI E3-5E-CC)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_5C0F
        RSSI: -65 dBm (0xbf)
> HCI Event: LE Meta Event (0x3e) plen 23                                  #86 [hci0] 2024-08-09 09:41:40.945177
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: E3:5E:CC:21:5C:0F (OUI E3-5E-CC)
        Data length: 11
        Company: not assigned (60552)
          Data: 001008a1256402
        RSSI: -66 dBm (0xbe)
@ MGMT Event: Device Found (0x0012) plen 54                           {0x0001} [hci0] 2024-08-09 09:41:40.945186
        LE Address: E3:5E:CC:21:5C:0F (OUI E3-5E-CC)
        RSSI: -66 dBm (0xbe)
        Flags: 0x00000000
        Data length: 40
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_5C0F
        Company: not assigned (60552)
          Data: 001008a1256402
> HCI Event: LE Meta Event (0x3e) plen 31                                  #87 [hci0] 2024-08-09 09:41:40.976782
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
        RSSI: -64 dBm (0xc0)
> HCI Event: LE Meta Event (0x3e) plen 12                                  #88 [hci0] 2024-08-09 09:41:40.977148
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 0
        RSSI: -63 dBm (0xc1)
@ MGMT Event: Device Found (0x0012) plen 33                           {0x0001} [hci0] 2024-08-09 09:41:40.977211
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -63 dBm (0xc1)
        Flags: 0x00000000
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
> HCI Event: LE Meta Event (0x3e) plen 43                                  #89 [hci0] 2024-08-09 09:41:40.995678
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:40.995697
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
> HCI Event: LE Meta Event (0x3e) plen 41                                  #90 [hci0] 2024-08-09 09:41:41.111254
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: E3:8E:C8:C1:98:9A (OUI E3-8E-C8)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_989A
        RSSI: -54 dBm (0xca)
> HCI Event: LE Meta Event (0x3e) plen 23                                  #91 [hci0] 2024-08-09 09:41:41.111893
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: E3:8E:C8:C1:98:9A (OUI E3-8E-C8)
        Data length: 11
        Company: not assigned (60552)
          Data: 009c07cc1c6202
        RSSI: -55 dBm (0xc9)
@ MGMT Event: Device Found (0x0012) plen 54                           {0x0001} [hci0] 2024-08-09 09:41:41.111903
        LE Address: E3:8E:C8:C1:98:9A (OUI E3-8E-C8)
        RSSI: -55 dBm (0xc9)
        Flags: 0x00000000
        Data length: 40
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_989A
        Company: not assigned (60552)
          Data: 009c07cc1c6202
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #92 [hci0] 2024-08-09 09:41:41.125134
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                #93 [hci0] 2024-08-09 09:41:41.126668
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #94 [hci0] 2024-08-09 09:41:41.126686
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                #95 [hci0] 2024-08-09 09:41:41.127017
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 43                                  #96 [hci0] 2024-08-09 09:41:41.197050
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:41.197064
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
> HCI Event: LE Meta Event (0x3e) plen 43                                  #97 [hci0] 2024-08-09 09:41:41.215740
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: C3:36:35:30:61:77 (Static)
        Data length: 31
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): GVH5104_6177
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Company: Nokia Mobile Phones (1)
          Data: 010103237e2b
        RSSI: -62 dBm (0xc2)
> HCI Event: LE Meta Event (0x3e) plen 43                                  #98 [hci0] 2024-08-09 09:41:41.234530
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -56 dBm (0xc8)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:41.234541
        LE Address: C3:36:35:30:61:77 (Static)
        RSSI: -62 dBm (0xc2)
        Flags: 0x00000000
        Data length: 31
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): GVH5104_6177
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Company: Nokia Mobile Phones (1)
          Data: 010103237e2b
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:41.234545
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -56 dBm (0xc8)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 31                                  #99 [hci0] 2024-08-09 09:41:41.297387
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
        RSSI: -63 dBm (0xc1)
> HCI Event: LE Meta Event (0x3e) plen 12                                 #100 [hci0] 2024-08-09 09:41:41.298009
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 0
        RSSI: -64 dBm (0xc0)
@ MGMT Event: Device Found (0x0012) plen 33                           {0x0001} [hci0] 2024-08-09 09:41:41.298017
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -64 dBm (0xc0)
        Flags: 0x00000000
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
> HCI Event: LE Meta Event (0x3e) plen 41                                 #101 [hci0] 2024-08-09 09:41:41.372538
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        RSSI: -48 dBm (0xd0)
> HCI Event: LE Meta Event (0x3e) plen 34                                 #102 [hci0] 2024-08-09 09:41:41.373312
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -48 dBm (0xd0)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-09 09:41:41.373319
        LE Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        RSSI: -48 dBm (0xd0)
        Flags: 0x00000000
        Data length: 51
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        Name (complete): Cerbo GX HQ2312AG767
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                    #103 [hci0] 2024-08-09 09:41:41.397136
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                               #104 [hci0] 2024-08-09 09:41:41.399171
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                    #105 [hci0] 2024-08-09 09:41:41.399194
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                               #106 [hci0] 2024-08-09 09:41:41.399523
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 31                                 #107 [hci0] 2024-08-09 09:41:41.428021
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
        RSSI: -64 dBm (0xc0)
> HCI Event: LE Meta Event (0x3e) plen 12                                 #108 [hci0] 2024-08-09 09:41:41.428402
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 0
        RSSI: -65 dBm (0xbf)
@ MGMT Event: Device Found (0x0012) plen 33                           {0x0001} [hci0] 2024-08-09 09:41:41.428410
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -65 dBm (0xbf)
        Flags: 0x00000000
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
> HCI Event: LE Meta Event (0x3e) plen 43                                 #109 [hci0] 2024-08-09 09:41:41.440224
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -53 dBm (0xcb)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:41.440241
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -53 dBm (0xcb)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 43                                 #110 [hci0] 2024-08-09 09:41:41.511261
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:41.511281
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
> HCI Event: LE Meta Event (0x3e) plen 41                                 #111 [hci0] 2024-08-09 09:41:41.524148
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: E3:60:59:21:80:65 (OUI E3-60-59)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_8065
        RSSI: -63 dBm (0xc1)
> HCI Event: LE Meta Event (0x3e) plen 23                                 #112 [hci0] 2024-08-09 09:41:41.524829
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: E3:60:59:21:80:65 (OUI E3-60-59)
        Data length: 11
        Company: not assigned (60552)
          Data: 00b1fcc50c5d02
        RSSI: -64 dBm (0xc0)
@ MGMT Event: Device Found (0x0012) plen 54                           {0x0001} [hci0] 2024-08-09 09:41:41.524838
        LE Address: E3:60:59:21:80:65 (OUI E3-60-59)
        RSSI: -64 dBm (0xc0)
        Flags: 0x00000000
        Data length: 40
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_8065
        Company: not assigned (60552)
          Data: 00b1fcc50c5d02
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                    #113 [hci0] 2024-08-09 09:41:41.633138
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                               #114 [hci0] 2024-08-09 09:41:41.635427
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                    #115 [hci0] 2024-08-09 09:41:41.635450
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                               #116 [hci0] 2024-08-09 09:41:41.635777
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 43                                 #117 [hci0] 2024-08-09 09:41:41.656977
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:41.657004
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 41                                 #118 [hci0] 2024-08-09 09:41:41.684405
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        RSSI: -42 dBm (0xd6)
> HCI Event: LE Meta Event (0x3e) plen 34                                 #119 [hci0] 2024-08-09 09:41:41.685233
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -42 dBm (0xd6)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-09 09:41:41.685241
        LE Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        RSSI: -42 dBm (0xd6)
        Flags: 0x00000000
        Data length: 51
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        Name (complete): Cerbo GX HQ2312AG767
> HCI Event: LE Meta Event (0x3e) plen 43                                 #120 [hci0] 2024-08-09 09:41:41.715693
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:41.715710
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                    #121 [hci0] 2024-08-09 09:41:41.857135
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                               #122 [hci0] 2024-08-09 09:41:41.859176
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                    #123 [hci0] 2024-08-09 09:41:41.859191
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                               #124 [hci0] 2024-08-09 09:41:41.859601
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 43                                 #125 [hci0] 2024-08-09 09:41:41.868973
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -53 dBm (0xcb)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:41.868980
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -53 dBm (0xcb)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 37                                 #126 [hci0] 2024-08-09 09:41:41.913295
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: A4:C1:38:05:C7:A1 (Telink Semiconductor (Taipei) Co. Ltd.)
        Data length: 25
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Name (complete): Govee_H5074_C7A1
        RSSI: -61 dBm (0xc3)
> HCI Event: LE Meta Event (0x3e) plen 39                                 #127 [hci0] 2024-08-09 09:41:41.914278
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: A4:C1:38:05:C7:A1 (Telink Semiconductor (Taipei) Co. Ltd.)
        Data length: 27
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: -62 dB
        RSSI: -61 dBm (0xc3)
@ MGMT Event: Device Found (0x0012) plen 66                           {0x0001} [hci0] 2024-08-09 09:41:41.914284
        LE Address: A4:C1:38:05:C7:A1 (Telink Semiconductor (Taipei) Co. Ltd.)
        RSSI: -61 dBm (0xc3)
        Flags: 0x00000000
        Data length: 52
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Name (complete): Govee_H5074_C7A1
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: -62 dB
> HCI Event: LE Meta Event (0x3e) plen 41                                 #128 [hci0] 2024-08-09 09:41:41.947269
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: E3:5E:CC:21:5C:0F (OUI E3-5E-CC)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_5C0F
        RSSI: -61 dBm (0xc3)
> HCI Event: LE Meta Event (0x3e) plen 41                                 #129 [hci0] 2024-08-09 09:41:42.003168
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        RSSI: -49 dBm (0xcf)
@ MGMT Event: Device Found (0x0012) plen 43                           {0x0001} [hci0] 2024-08-09 09:41:42.003187
        LE Address: E3:5E:CC:21:5C:0F (OUI E3-5E-CC)
        RSSI: -61 dBm (0xc3)
        Flags: 0x00000000
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_5C0F
> HCI Event: LE Meta Event (0x3e) plen 34                                 #130 [hci0] 2024-08-09 09:41:42.003991
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -49 dBm (0xcf)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-09 09:41:42.003998
        LE Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        RSSI: -49 dBm (0xcf)
        Flags: 0x00000000
        Data length: 51
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        Name (complete): Cerbo GX HQ2312AG767
> HCI Event: LE Meta Event (0x3e) plen 43                                 #131 [hci0] 2024-08-09 09:41:42.025792
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:42.025806
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                    #132 [hci0] 2024-08-09 09:41:42.069134
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                               #133 [hci0] 2024-08-09 09:41:42.070430
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                    #134 [hci0] 2024-08-09 09:41:42.070448
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                               #135 [hci0] 2024-08-09 09:41:42.070779
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 43                                 #136 [hci0] 2024-08-09 09:41:42.080228
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -55 dBm (0xc9)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:42.080235
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -55 dBm (0xc9)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 41                                 #137 [hci0] 2024-08-09 09:41:42.112657
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: E3:8E:C8:C1:98:9A (OUI E3-8E-C8)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_989A
        RSSI: -55 dBm (0xc9)
> HCI Event: LE Meta Event (0x3e) plen 39                                 #138 [hci0] 2024-08-09 09:41:42.113336
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: E3:8E:C8:C1:98:9A (OUI E3-8E-C8)
        Data length: 27
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 29776.39066
          TX power: -62 dB
        RSSI: -55 dBm (0xc9)
@ MGMT Event: Device Found (0x0012) plen 70                           {0x0001} [hci0] 2024-08-09 09:41:42.113340
        LE Address: E3:8E:C8:C1:98:9A (OUI E3-8E-C8)
        RSSI: -55 dBm (0xc9)
        Flags: 0x00000000
        Data length: 56
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 3 entries
          Device Information (0x180a)
          Dialog Semiconductor GmbH (0xfef5)
          Unknown (0xec88)
        Name (complete): Govee_H5074_989A
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 29776.39066
          TX power: -62 dB
> HCI Event: LE Meta Event (0x3e) plen 31                                 #139 [hci0] 2024-08-09 09:41:42.142397
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
        RSSI: -63 dBm (0xc1)
> HCI Event: LE Meta Event (0x3e) plen 12                                 #140 [hci0] 2024-08-09 09:41:42.142939
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 0
        RSSI: -63 dBm (0xc1)
@ MGMT Event: Device Found (0x0012) plen 33                           {0x0001} [hci0] 2024-08-09 09:41:42.142945
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -63 dBm (0xc1)
        Flags: 0x00000000
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
> HCI Event: LE Meta Event (0x3e) plen 41                                 #141 [hci0] 2024-08-09 09:41:42.161274
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        RSSI: -42 dBm (0xd6)
> HCI Event: LE Meta Event (0x3e) plen 34                                 #142 [hci0] 2024-08-09 09:41:42.162057
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -42 dBm (0xd6)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-09 09:41:42.162063
        LE Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        RSSI: -42 dBm (0xd6)
        Flags: 0x00000000
        Data length: 51
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        Name (complete): Cerbo GX HQ2312AG767
> HCI Event: LE Meta Event (0x3e) plen 43                                 #143 [hci0] 2024-08-09 09:41:42.237045
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:42.237065
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                    #144 [hci0] 2024-08-09 09:41:42.281128
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                               #145 [hci0] 2024-08-09 09:41:42.282936
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                    #146 [hci0] 2024-08-09 09:41:42.282953
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                               #147 [hci0] 2024-08-09 09:41:42.283290
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 37                                 #148 [hci0] 2024-08-09 09:41:42.312868
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: A4:C1:38:D5:A3:3B (Telink Semiconductor (Taipei) Co. Ltd.)
        Data length: 25
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Name (complete): Govee_H5074_A33B
        RSSI: -55 dBm (0xc9)
> HCI Event: LE Meta Event (0x3e) plen 39                                 #149 [hci0] 2024-08-09 09:41:42.312872
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: A4:C1:38:D5:A3:3B (Telink Semiconductor (Taipei) Co. Ltd.)
        Data length: 27
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: -62 dB
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 66                           {0x0001} [hci0] 2024-08-09 09:41:42.312891
        LE Address: A4:C1:38:D5:A3:3B (Telink Semiconductor (Taipei) Co. Ltd.)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000000
        Data length: 52
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Name (complete): Govee_H5074_A33B
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: -62 dB
> HCI Event: LE Meta Event (0x3e) plen 31                                 #150 [hci0] 2024-08-09 09:41:42.335023
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
        RSSI: -61 dBm (0xc3)
> HCI Event: LE Meta Event (0x3e) plen 12                                 #151 [hci0] 2024-08-09 09:41:42.335504
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 0
        RSSI: -64 dBm (0xc0)
@ MGMT Event: Device Found (0x0012) plen 33                           {0x0001} [hci0] 2024-08-09 09:41:42.335528
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -64 dBm (0xc0)
        Flags: 0x00000000
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
> HCI Event: LE Meta Event (0x3e) plen 43                                 #152 [hci0] 2024-08-09 09:41:42.345684
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
        RSSI: -60 dBm (0xc4)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:42.345706
        LE Address: 15:EF:7B:19:B3:7B (Non-Resolvable)
        RSSI: -60 dBm (0xc4)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 01092022c874a4b94f1649d5954ae21f68741c374816748b621901
> HCI Event: LE Meta Event (0x3e) plen 43                                 #153 [hci0] 2024-08-09 09:41:42.392741
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 34:AB:86:59:39:54 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
        RSSI: -54 dBm (0xca)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-09 09:41:42.392763
        LE Address: 34:AB:86:59:39:54 (Non-Resolvable)
        RSSI: -54 dBm (0xca)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 0109202297d51a861c1a84c8eb72b0a73801769f5281cae1f7f23d
> HCI Event: LE Meta Event (0x3e) plen 41                                 #154 [hci0] 2024-08-09 09:41:42.474408
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 29
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        RSSI: -42 dBm (0xd6)
> HCI Event: LE Meta Event (0x3e) plen 34                                 #155 [hci0] 2024-08-09 09:41:42.475242
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -42 dBm (0xd6)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-09 09:41:42.475251
        LE Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        RSSI: -42 dBm (0xd6)
        Flags: 0x00000000
        Data length: 51
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        128-bit Service UUIDs (complete): 1 entry
          Vendor specific
        Company: Victron Energy BV (737)
          Data: 10000ac0
        Name (complete): Cerbo GX HQ2312AG767
@ MGMT Command: Stop Discovery (0x0024) plen 1                        {0x0001} [hci0] 2024-08-09 09:41:42.487086
        Address type: 0x06
          LE Public
          LE Random
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                    #156 [hci0] 2024-08-09 09:41:42.487124
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                               #157 [hci0] 2024-08-09 09:41:42.489186
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
@ MGMT Event: Command Complete (0x0001) plen 4                        {0x0001} [hci0] 2024-08-09 09:41:42.489203
      Stop Discovery (0x0024) plen 1
        Status: Success (0x00)
        Address type: 0x06
          LE Public
          LE Random
@ MGMT Event: Discovering (0x0013) plen 2                             {0x0001} [hci0] 2024-08-09 09:41:42.489206
        Address type: 0x06
          LE Public
          LE Random
        Discovery: Disabled (0x00)
```

Here's the matching output from my program running at the same time:

```
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1411): Connected to D-Bus as ":1.5752"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(973): org.freedesktop.DBus.ObjectManager: GetManagedObjects
  Message Type: method_return
     Signature: a{oa{sa{sv}}}
   Destination: :1.5752
        Sender: :1.6
       Object Path: /org/bluez
            String: org.freedesktop.DBus.Introspectable
            String: org.bluez.AgentManager1
            String: org.bluez.ProfileManager1
            String: org.bluez.HealthManager1
       Object Path: /org/bluez/hci0
            String: org.freedesktop.DBus.Introspectable
            String: org.bluez.Adapter1
            String: org.freedesktop.DBus.Properties
            String: org.bluez.GattManager1
            String: org.bluez.Media1
            String: org.bluez.NetworkServer1
            String: org.bluez.LEAdvertisingManager1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1415): Bluetooth Adapter: /org/bluez/hci0
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(916): org.bluez.Adapter1: SetDiscoveryFilter
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(941): org.bluez.Adapter1: StartDiscovery
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "s"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/freedesktop/DBus
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: NameAcquired
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1485):     String: :1.5752
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: method_returnSignature: ""
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: 34:AB:86:59:39:54 (String)
                 AddressType: random (String)
                       Alias: 34-AB-86-59-39-54 (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -62 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: 15:EF:7B:19:B3:7B (String)
                 AddressType: random (String)
                       Alias: 15-EF-7B-19-B3-7B (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -54 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_6C_2A_DF_E0_14_1A
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: 6C:2A:DF:E0:14:1A (String)
                 AddressType: public (String)
                        Name: HB-00147479 (String)
                       Alias: HB-00147479 (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -61 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
                     TxPower: 3 (Int16)
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -53 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_6C_2A_DF_E0_14_1A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -63 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_75_2E_2E_EB_6B_D5
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: 75:2E:2E:EB:6B:D5 (String)
                 AddressType: random (String)
                        Name: Cerbo GX HQ2312AG767 (String)
                       Alias: Cerbo GX HQ2312AG767 (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -48 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -55 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_6C_2A_DF_E0_14_1A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -64 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_75_2E_2E_EB_6B_D5
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -49 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_A4_C1_38_05_C7_A1
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: A4:C1:38:05:C7:A1 (String)
                 AddressType: public (String)
                        Name: Govee_H5074_C7A1 (String)
                       Alias: Govee_H5074_C7A1 (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -64 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_E3_5E_CC_21_5C_0F
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: E3:5E:CC:21:5C:0F (String)
                 AddressType: public (String)
                        Name: Govee_H5074_5C0F (String)
                       Alias: Govee_H5074_5C0F (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -62 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_E3_60_59_23_14_7D
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: E3:60:59:23:14:7D (String)
                 AddressType: public (String)
                        Name: Govee_H5074_147D (String)
                       Alias: Govee_H5074_147D (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -68 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -55 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_75_2E_2E_EB_6B_D5
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -42 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_E3_8E_C8_C1_98_9A
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: E3:8E:C8:C1:98:9A (String)
                 AddressType: public (String)
                        Name: Govee_H5074_989A (String)
                       Alias: Govee_H5074_989A (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -49 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -65 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_A4_C1_38_D5_A3_3B
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: A4:C1:38:D5:A3:3B (String)
                 AddressType: public (String)
                        Name: Govee_H5074_A33B (String)
                       Alias: Govee_H5074_A33B (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -54 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_75_2E_2E_EB_6B_D5
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -42 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -55 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_6C_2A_DF_E0_14_1A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -62 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_C2_35_33_30_25_50
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: C2:35:33:30:25:50 (String)
                 AddressType: random (String)
                        Name: GVH5100_2550 (String)
                       Alias: GVH5100_2550 (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -44 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -64 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_6C_2A_DF_E0_14_1A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -65 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_D0_35_33_33_44_03
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: D0:35:33:33:44:03 (String)
                 AddressType: random (String)
                        Name: GVH5105_4403 (String)
                       Alias: GVH5105_4403 (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -53 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_75_2E_2E_EB_6B_D5
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -48 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_A4_C1_38_0D_3B_10
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: A4:C1:38:0D:3B:10 (String)
                 AddressType: public (String)
                        Name: GVH5177_3B10 (String)
                       Alias: GVH5177_3B10 (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -64 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -55 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_C4_A0_4D_54_0E_11
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: C4:A0:4D:54:0E:11 (String)
                 AddressType: random (String)
                       Alias: C4-A0-4D-54-0E-11 (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -64 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_E3_5E_CC_21_5C_0F
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -66 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_6C_2A_DF_E0_14_1A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -63 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_E3_8E_C8_C1_98_9A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -55 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_C3_36_35_30_61_77
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: C3:36:35:30:61:77 (String)
                 AddressType: random (String)
                        Name: GVH5104_6177 (String)
                       Alias: GVH5104_6177 (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -62 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -56 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_6C_2A_DF_E0_14_1A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -64 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_75_2E_2E_EB_6B_D5
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -48 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_6C_2A_DF_E0_14_1A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -65 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -53 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: InterfacesAdded
       Object Path: /org/bluez/hci0/dev_E3_60_59_21_80_65
                String: org.freedesktop.DBus.Introspectable
                String: org.bluez.Device1
                     Address: E3:60:59:21:80:65 (String)
                 AddressType: public (String)
                        Name: Govee_H5074_8065 (String)
                       Alias: Govee_H5074_8065 (String)
                      Paired: 0 (Boolean)
                      Bonded: 0 (Boolean)
                     Trusted: 0 (Boolean)
                     Blocked: 0 (Boolean)
               LegacyPairing: 0 (Boolean)
                        RSSI: -64 (Int16)
                   Connected: 0 (Boolean)
                       UUIDs: Array TODO: Decode
                     Adapter: /org/bluez/hci0 (ObjectPath)
            ManufacturerData: Array TODO: Decode
            ServicesResolved: 0 (Boolean)
                String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_75_2E_2E_EB_6B_D5
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -42 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -53 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_A4_C1_38_05_C7_A1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -61 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_E3_5E_CC_21_5C_0F
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -61 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_75_2E_2E_EB_6B_D5
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -49 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_15_EF_7B_19_B3_7B
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -54 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_34_AB_86_59_39_54
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -55 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1457): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1467):   Path: /org/bluez/hci0/dev_E3_8E_C8_C1_98_9A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1468):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1469):   Member: PropertiesChanged
       Object Path: org.bluez.Device1
                    RSSI:  Value: -55 (Int16)
        ManufacturerData:  Value: Array TODO: Decode
```

The process of finding the correct adapter doesn't show up in btmon at all. 

The MGMT Command: Start Service Discovery initiates a sequence of HCI Commands and Events.
```
@ MGMT Command: Start Service Discovery (0x003a) plen 4               {0x0001} [hci0] 2024-08-09 09:41:39.255301
        Address type: 0x06
          LE Public
          LE Random
        RSSI: invalid (0x7f)
        UUIDs: 0
< HCI Command: LE Set Random Address (0x08|0x0005) plen 6                   #1 [hci0] 2024-08-09 09:41:39.255376
        Address: 1D:43:5A:0A:81:1C (Non-Resolvable)
> HCI Event: Command Complete (0x0e) plen 4                                 #2 [hci0] 2024-08-09 09:41:39.255666
      LE Set Random Address (0x08|0x0005) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Parameters (0x08|0x000b) plen 7                  #3 [hci0] 2024-08-09 09:41:39.255684
        Type: Active (0x01)
        Interval: 11.250 msec (0x0012)
        Window: 11.250 msec (0x0012)
        Own address type: Random (0x01)
        Filter policy: Accept all advertisement (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                 #4 [hci0] 2024-08-09 09:41:39.255971
      LE Set Scan Parameters (0x08|0x000b) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                      #5 [hci0] 2024-08-09 09:41:39.255980
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                 #6 [hci0] 2024-08-09 09:41:39.256282
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
@ MGMT Event: Command Complete (0x0001) plen 4                        {0x0001} [hci0] 2024-08-09 09:41:39.256291
      Start Service Discovery (0x003a) plen 1
        Status: Success (0x00)
        Address type: 0x06
          LE Public
          LE Random
@ MGMT Event: Discovering (0x0013) plen 2                             {0x0001} [hci0] 2024-08-09 09:41:39.256294

        Address type: 0x06
          LE Public
          LE Random
        Discovery: Enabled (0x01)
```

# These three sites helped me figure out how to send an array of DICT entries
* https://stackoverflow.com/questions/29973486/d-bus-how-to-create-and-send-a-dict
* https://git.kernel.org/pub/scm/network/connman/connman.git/tree/gdbus/client.c#n667
* https://android.googlesource.com/platform/external/wpa_supplicant_8/+/master/wpa_supplicant/dbus/dbus_dict_helpers.c
