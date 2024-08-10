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
= Note: Linux version 6.6.31+rpt-rpi-2712 (aarch64)                                   2024-08-10 10:32:01.476857
= Note: Bluetooth subsystem version 2.22                                              2024-08-10 10:32:01.476859
= New Index: 2C:CF:67:0B:78:71 (Primary,UART,hci0)                             [hci0] 2024-08-10 10:32:01.476861
= Open Index: 2C:CF:67:0B:78:71                                                [hci0] 2024-08-10 10:32:01.476861
= Index Info: 2C:CF:67:0B:78:71 (Cypress Semiconductor)                        [hci0] 2024-08-10 10:32:01.476861
@ MGMT Open: bluetoothd (privileged) version 1.22                            {0x0001} 2024-08-10 10:32:01.476862
@ MGMT Command: Start Service Discovery (0x003a) plen 4               {0x0001} [hci0] 2024-08-10 10:32:26.117695
        Address type: 0x06
          LE Public
          LE Random
        RSSI: invalid (0x7f)
        UUIDs: 0
< HCI Command: LE Set Random Address (0x08|0x0005) plen 6                   #1 [hci0] 2024-08-10 10:32:26.117780
        Address: 3B:EA:E0:96:31:0F (Non-Resolvable)
> HCI Event: Command Complete (0x0e) plen 4                                 #2 [hci0] 2024-08-10 10:32:26.118091
      LE Set Random Address (0x08|0x0005) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Parameters (0x08|0x000b) plen 7                  #3 [hci0] 2024-08-10 10:32:26.118104
        Type: Active (0x01)
        Interval: 11.250 msec (0x0012)
        Window: 11.250 msec (0x0012)
        Own address type: Random (0x01)
        Filter policy: Accept all advertisement (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                 #4 [hci0] 2024-08-10 10:32:26.118370
      LE Set Scan Parameters (0x08|0x000b) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                      #5 [hci0] 2024-08-10 10:32:26.118379
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                 #6 [hci0] 2024-08-10 10:32:26.118727
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
@ MGMT Event: Command Complete (0x0001) plen 4                        {0x0001} [hci0] 2024-08-10 10:32:26.118736
      Start Service Discovery (0x003a) plen 1
        Status: Success (0x00)
        Address type: 0x06
          LE Public
          LE Random
@ MGMT Event: Discovering (0x0013) plen 2                             {0x0001} [hci0] 2024-08-10 10:32:26.118739
        Address type: 0x06
          LE Public
          LE Random
        Discovery: Enabled (0x01)
> HCI Event: LE Meta Event (0x3e) plen 43                                   #7 [hci0] 2024-08-10 10:32:26.152383
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 1B:9D:D6:26:B4:52 (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 010920223ad83458f8f875121f13de05784f796efd1aa7eb2cf13b
        RSSI: -61 dBm (0xc3)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-10 10:32:26.152401
        LE Address: 1B:9D:D6:26:B4:52 (Non-Resolvable)
        RSSI: -61 dBm (0xc3)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 010920223ad83458f8f875121f13de05784f796efd1aa7eb2cf13b
> HCI Event: LE Meta Event (0x3e) plen 43                                   #8 [hci0] 2024-08-10 10:32:26.172249
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 23:C8:DC:BA:58:2D (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 010920220054e48554f53f89b032205eaae65414d06f37edd2da3c
        RSSI: -64 dBm (0xc0)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-10 10:32:26.172266
        LE Address: 23:C8:DC:BA:58:2D (Non-Resolvable)
        RSSI: -64 dBm (0xc0)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 010920220054e48554f53f89b032205eaae65414d06f37edd2da3c
> HCI Event: LE Meta Event (0x3e) plen 31                                   #9 [hci0] 2024-08-10 10:32:26.197722
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
        RSSI: -66 dBm (0xbe)
> HCI Event: LE Meta Event (0x3e) plen 12                                  #10 [hci0] 2024-08-10 10:32:26.198349
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 0
        RSSI: -67 dBm (0xbd)
@ MGMT Event: Device Found (0x0012) plen 33                           {0x0001} [hci0] 2024-08-10 10:32:26.198355
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -67 dBm (0xbd)
        Flags: 0x00000000
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
> HCI Event: LE Meta Event (0x3e) plen 41                                  #11 [hci0] 2024-08-10 10:32:26.297800
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
        RSSI: -63 dBm (0xc1)
> HCI Event: LE Meta Event (0x3e) plen 41                                  #12 [hci0] 2024-08-10 10:32:26.302593
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
@ MGMT Event: Device Found (0x0012) plen 43                           {0x0001} [hci0] 2024-08-10 10:32:26.302603
        LE Address: E3:5E:CC:21:5C:0F (OUI E3-5E-CC)
        RSSI: -63 dBm (0xc1)
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
> HCI Event: LE Meta Event (0x3e) plen 34                                  #13 [hci0] 2024-08-10 10:32:26.303672
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -47 dBm (0xd1)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-10 10:32:26.303688
        LE Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        RSSI: -47 dBm (0xd1)
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
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #14 [hci0] 2024-08-10 10:32:26.357131
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                #15 [hci0] 2024-08-10 10:32:26.358624
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #16 [hci0] 2024-08-10 10:32:26.358641
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                #17 [hci0] 2024-08-10 10:32:26.358974
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 43                                  #18 [hci0] 2024-08-10 10:32:26.387270
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 23:C8:DC:BA:58:2D (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 010920220054e48554f53f89b032205eaae65414d06f37edd2da3c
        RSSI: -56 dBm (0xc8)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-10 10:32:26.387284
        LE Address: 23:C8:DC:BA:58:2D (Non-Resolvable)
        RSSI: -56 dBm (0xc8)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 010920220054e48554f53f89b032205eaae65414d06f37edd2da3c
> HCI Event: LE Meta Event (0x3e) plen 31                                  #19 [hci0] 2024-08-10 10:32:26.390816
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
> HCI Event: LE Meta Event (0x3e) plen 12                                  #20 [hci0] 2024-08-10 10:32:26.391351
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        Data length: 0
        RSSI: -65 dBm (0xbf)
@ MGMT Event: Device Found (0x0012) plen 33                           {0x0001} [hci0] 2024-08-10 10:32:26.391358
        LE Address: 6C:2A:DF:E0:14:1A (OUI 6C-2A-DF)
        RSSI: -65 dBm (0xbf)
        Flags: 0x00000000
        Data length: 19
        Flags: 0x06
          LE General Discoverable Mode
          BR/EDR Not Supported
        Name (complete): HB-00147479
        TX power: 3 dBm
> HCI Event: LE Meta Event (0x3e) plen 43                                  #21 [hci0] 2024-08-10 10:32:26.452219
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Connectable undirected - ADV_IND (0x00)
        Address type: Public (0x00)
        Address: A4:C1:38:DC:CC:3D (Telink Semiconductor (Taipei) Co. Ltd.)
        Data length: 31
        Name (complete): GVH5174_CC3D
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Flags: 0x05
          LE Limited Discoverable Mode
          BR/EDR Not Supported
        Company: Nokia Mobile Phones (1)
          Data: 010102fc8f44
        RSSI: -51 dBm (0xcd)
> HCI Event: LE Meta Event (0x3e) plen 39                                  #22 [hci0] 2024-08-10 10:32:26.453091
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: A4:C1:38:DC:CC:3D (Telink Semiconductor (Taipei) Co. Ltd.)
        Data length: 27
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: -62 dB
        RSSI: -51 dBm (0xcd)
@ MGMT Event: Device Found (0x0012) plen 72                           {0x0001} [hci0] 2024-08-10 10:32:26.453098
        LE Address: A4:C1:38:DC:CC:3D (Telink Semiconductor (Taipei) Co. Ltd.)
        RSSI: -51 dBm (0xcd)
        Flags: 0x00000000
        Data length: 58
        Name (complete): GVH5174_CC3D
        16-bit Service UUIDs (complete): 1 entry
          Unknown (0xec88)
        Flags: 0x05
          LE Limited Discoverable Mode
          BR/EDR Not Supported
        Company: Nokia Mobile Phones (1)
          Data: 010102fc8f44
        Company: Apple, Inc. (76)
          Type: iBeacon (2)
          UUID: 57485f53-4b43-4f52-5f49-4c4c45544e49
          Version: 30032.65522
          TX power: -62 dB
> HCI Event: LE Meta Event (0x3e) plen 43                                  #23 [hci0] 2024-08-10 10:32:26.478920
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 4C:E6:63:CE:23:48 (Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 010920021308eb176b3cc0e19625876879c2608ef1c8a530a133e3
        RSSI: -62 dBm (0xc2)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-10 10:32:26.478939
        LE Address: 4C:E6:63:CE:23:48 (Resolvable)
        RSSI: -62 dBm (0xc2)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 010920021308eb176b3cc0e19625876879c2608ef1c8a530a133e3
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #24 [hci0] 2024-08-10 10:32:26.589136
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                #25 [hci0] 2024-08-10 10:32:26.591136
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #26 [hci0] 2024-08-10 10:32:26.591153
        Scanning: Enabled (0x01)
        Filter duplicates: Enabled (0x01)
> HCI Event: Command Complete (0x0e) plen 4                                #27 [hci0] 2024-08-10 10:32:26.591494
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
> HCI Event: LE Meta Event (0x3e) plen 41                                  #28 [hci0] 2024-08-10 10:32:26.610105
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
> HCI Event: LE Meta Event (0x3e) plen 34                                  #29 [hci0] 2024-08-10 10:32:26.610957
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Random (0x01)
        Address: 75:2E:2E:EB:6B:D5 (Resolvable)
        Data length: 22
        Name (complete): Cerbo GX HQ2312AG767
        RSSI: -48 dBm (0xd0)
@ MGMT Event: Device Found (0x0012) plen 65                           {0x0001} [hci0] 2024-08-10 10:32:26.610963
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
> HCI Event: LE Meta Event (0x3e) plen 41                                  #30 [hci0] 2024-08-10 10:32:26.648578
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
        RSSI: -51 dBm (0xcd)
> HCI Event: LE Meta Event (0x3e) plen 23                                  #31 [hci0] 2024-08-10 10:32:26.649142
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Scan response - SCAN_RSP (0x04)
        Address type: Public (0x00)
        Address: E3:8E:C8:C1:98:9A (OUI E3-8E-C8)
        Data length: 11
        Company: not assigned (60552)
          Data: 00ab07301c6302
        RSSI: -52 dBm (0xcc)
@ MGMT Event: Device Found (0x0012) plen 54                           {0x0001} [hci0] 2024-08-10 10:32:26.649149
        LE Address: E3:8E:C8:C1:98:9A (OUI E3-8E-C8)
        RSSI: -52 dBm (0xcc)
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
          Data: 00ab07301c6302
> HCI Event: LE Meta Event (0x3e) plen 43                                  #32 [hci0] 2024-08-10 10:32:26.688241
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 4C:E6:63:CE:23:48 (Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 010920021308eb176b3cc0e19625876879c2608ef1c8a530a133e3
        RSSI: -62 dBm (0xc2)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-10 10:32:26.688259
        LE Address: 4C:E6:63:CE:23:48 (Resolvable)
        RSSI: -62 dBm (0xc2)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 010920021308eb176b3cc0e19625876879c2608ef1c8a530a133e3
> HCI Event: LE Meta Event (0x3e) plen 43                                  #33 [hci0] 2024-08-10 10:32:26.697287
      LE Advertising Report (0x02)
        Num reports: 1
        Event type: Non connectable undirected - ADV_NONCONN_IND (0x03)
        Address type: Random (0x01)
        Address: 23:C8:DC:BA:58:2D (Non-Resolvable)
        Data length: 31
        Company: Microsoft (6)
          Data: 010920220054e48554f53f89b032205eaae65414d06f37edd2da3c
        RSSI: -56 dBm (0xc8)
@ MGMT Event: Device Found (0x0012) plen 45                           {0x0001} [hci0] 2024-08-10 10:32:26.697301
        LE Address: 23:C8:DC:BA:58:2D (Non-Resolvable)
        RSSI: -56 dBm (0xc8)
        Flags: 0x00000004
          Not Connectable
        Data length: 31
        Company: Microsoft (6)
          Data: 010920220054e48554f53f89b032205eaae65414d06f37edd2da3c
@ MGMT Command: Stop Discovery (0x0024) plen 1                        {0x0001} [hci0] 2024-08-10 10:32:26.710809
        Address type: 0x06
          LE Public
          LE Random
< HCI Command: LE Set Scan Enable (0x08|0x000c) plen 2                     #34 [hci0] 2024-08-10 10:32:26.710834
        Scanning: Disabled (0x00)
        Filter duplicates: Disabled (0x00)
> HCI Event: Command Complete (0x0e) plen 4                                #35 [hci0] 2024-08-10 10:32:26.712393
      LE Set Scan Enable (0x08|0x000c) ncmd 1
        Status: Success (0x00)
@ MGMT Event: Command Complete (0x0001) plen 4                        {0x0001} [hci0] 2024-08-10 10:32:26.712409
      Stop Discovery (0x0024) plen 1
        Status: Success (0x00)
        Address type: 0x06
          LE Public
          LE Random
@ MGMT Event: Discovering (0x0013) plen 2                             {0x0001} [hci0] 2024-08-10 10:32:26.712413
        Address type: 0x06
          LE Public
          LE Random
        Discovery: Disabled (0x00)
```

Here's the matching output from my program running at the same time:

```
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1026): Connected to D-Bus as ":1.7693"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(433): /: org.freedesktop.DBus.ObjectManager: GetManagedObjects
  Message Type: method_return
     Signature: a{oa{sa{sv}}}
   Destination: :1.7693
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
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1030): Bluetooth Adapter: /org/bluez/hci0
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(318): /org/bluez/hci0: org.freedesktop.DBus.Properties: SetPowered: true
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(376): /org/bluez/hci0: org.bluez.Adapter1: SetDiscoveryFilter
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(401): /org/bluez/hci0: org.bluez.Adapter1: StartDiscovery
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "s"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /org/freedesktop/DBus
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: NameAcquired
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1099):     String: :1.7693
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: method_return     Signature: ""
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: InterfacesAdded
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(691):        Object Path: /org/bluez/hci0/dev_1B_9D_D6_26_B4_52
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Introspectable
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.bluez.Device1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Address: 1B:9D:D6:26:B4:52 (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                  AddressType: random (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        Alias: 1B-9D-D6-26-B4-52 (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Paired: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Bonded: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Trusted: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Blocked: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                LegacyPairing: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         RSSI: -61 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                    Connected: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        UUIDs:  (Invalid)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Adapter: /org/bluez/hci0 (ObjectPath)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ManufacturerData:  (DBUS_TYPE_DICT_ENTRY){ (UINT16: 0006) (DBUS_TYPE_VARIANT)[ (DBUS_TYPE_ARRAY){010920223ad83458f8f875121f13de05784f796efd1aa7eb2cf13b} ] }
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ServicesResolved: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: InterfacesAdded
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(691):        Object Path: /org/bluez/hci0/dev_23_C8_DC_BA_58_2D
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Introspectable
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.bluez.Device1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Address: 23:C8:DC:BA:58:2D (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                  AddressType: random (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        Alias: 23-C8-DC-BA-58-2D (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Paired: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Bonded: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Trusted: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Blocked: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                LegacyPairing: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         RSSI: -64 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                    Connected: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        UUIDs:  (Invalid)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Adapter: /org/bluez/hci0 (ObjectPath)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ManufacturerData:  (DBUS_TYPE_DICT_ENTRY){ (UINT16: 0006) (DBUS_TYPE_VARIANT)[ (DBUS_TYPE_ARRAY){010920220054e48554f53f89b032205eaae65414d06f37edd2da3c} ] }
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ServicesResolved: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: InterfacesAdded
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(691):        Object Path: /org/bluez/hci0/dev_6C_2A_DF_E0_14_1A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Introspectable
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.bluez.Device1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Address: 6C:2A:DF:E0:14:1A (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                  AddressType: public (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         Name: HB-00147479 (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        Alias: HB-00147479 (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Paired: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Bonded: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Trusted: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Blocked: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                LegacyPairing: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         RSSI: -67 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                    Connected: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        UUIDs:  (Invalid)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Adapter: /org/bluez/hci0 (ObjectPath)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      TxPower: 3 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ServicesResolved: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: InterfacesAdded
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(691):        Object Path: /org/bluez/hci0/dev_E3_5E_CC_21_5C_0F
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Introspectable
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.bluez.Device1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Address: E3:5E:CC:21:5C:0F (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                  AddressType: public (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         Name: Govee_H5074_5C0F (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        Alias: Govee_H5074_5C0F (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Paired: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Bonded: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Trusted: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Blocked: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                LegacyPairing: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         RSSI: -63 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                    Connected: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        UUIDs:  0000180a-0000-1000-8000-00805f9b34fb 0000fef5-0000-1000-8000-00805f9b34fb 0000ec88-0000-1000-8000-00805f9b34fb
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Adapter: /org/bluez/hci0 (ObjectPath)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ServicesResolved: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: InterfacesAdded
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(691):        Object Path: /org/bluez/hci0/dev_75_2E_2E_EB_6B_D5
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Introspectable
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.bluez.Device1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Address: 75:2E:2E:EB:6B:D5 (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                  AddressType: random (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         Name: Cerbo GX HQ2312AG767 (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        Alias: Cerbo GX HQ2312AG767 (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Paired: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Bonded: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Trusted: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Blocked: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                LegacyPairing: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         RSSI: -47 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                    Connected: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        UUIDs:  cd54c00a-880b-425b-b167-81ed6a15e913
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Adapter: /org/bluez/hci0 (ObjectPath)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ManufacturerData:  (DBUS_TYPE_DICT_ENTRY){ (UINT16: 02e1) (DBUS_TYPE_VARIANT)[ (DBUS_TYPE_ARRAY){10000ac0} ] }
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ServicesResolved: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /org/bluez/hci0/dev_23_C8_DC_BA_58_2D
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: PropertiesChanged
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(869):        Object Path: org.bluez.Device1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(881):                     RSSI: -56 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(881):         ManufacturerData:  (DBUS_TYPE_DICT_ENTRY){ (UINT16: 0006) (DBUS_TYPE_VARIANT)[ (DBUS_TYPE_ARRAY){010920220054e48554f53f89b032205eaae65414d06f37edd2da3c} ] }
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /org/bluez/hci0/dev_6C_2A_DF_E0_14_1A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: PropertiesChanged
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(869):        Object Path: org.bluez.Device1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(881):                     RSSI: -65 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: InterfacesAdded
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(691):        Object Path: /org/bluez/hci0/dev_A4_C1_38_DC_CC_3D
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Introspectable
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.bluez.Device1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Address: A4:C1:38:DC:CC:3D (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                  AddressType: public (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         Name: GVH5174_CC3D (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        Alias: GVH5174_CC3D (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Paired: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Bonded: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Trusted: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Blocked: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                LegacyPairing: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         RSSI: -51 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                    Connected: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        UUIDs:  0000ec88-0000-1000-8000-00805f9b34fb
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Adapter: /org/bluez/hci0 (ObjectPath)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ManufacturerData:  (DBUS_TYPE_DICT_ENTRY){ (UINT16: 0001) (DBUS_TYPE_VARIANT)[ (DBUS_TYPE_ARRAY){010102fc8f44} ] } (DBUS_TYPE_DICT_ENTRY){ (UINT16: 004c) (DBUS_TYPE_VARIANT)[ (DBUS_TYPE_ARRAY){0215494e54454c4c495f524f434b535f48575075f2ffc2} ] }
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ServicesResolved: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: InterfacesAdded
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(691):        Object Path: /org/bluez/hci0/dev_4C_E6_63_CE_23_48
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Introspectable
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.bluez.Device1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Address: 4C:E6:63:CE:23:48 (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                  AddressType: random (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        Alias: 4C-E6-63-CE-23-48 (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Paired: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Bonded: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Trusted: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Blocked: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                LegacyPairing: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         RSSI: -62 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                    Connected: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        UUIDs:  (Invalid)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Adapter: /org/bluez/hci0 (ObjectPath)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ManufacturerData:  (DBUS_TYPE_DICT_ENTRY){ (UINT16: 0006) (DBUS_TYPE_VARIANT)[ (DBUS_TYPE_ARRAY){010920021308eb176b3cc0e19625876879c2608ef1c8a530a133e3} ] }
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ServicesResolved: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "sa{sv}as"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /org/bluez/hci0/dev_75_2E_2E_EB_6B_D5
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus.Properties
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: PropertiesChanged
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(869):        Object Path: org.bluez.Device1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(881):                     RSSI: -48 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(881):         ManufacturerData:  (DBUS_TYPE_DICT_ENTRY){ (UINT16: 02e1) (DBUS_TYPE_VARIANT)[ (DBUS_TYPE_ARRAY){10000ac0} ] }
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1072): Got message:       Type: signal    Signature: "oa{sa{sv}}"
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1082):   Path: /
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1083):   Interface: org.freedesktop.DBus.ObjectManager
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(1084):   Member: InterfacesAdded
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(691):        Object Path: /org/bluez/hci0/dev_E3_8E_C8_C1_98_9A
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Introspectable
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.bluez.Device1
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Address: E3:8E:C8:C1:98:9A (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                  AddressType: public (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         Name: Govee_H5074_989A (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        Alias: Govee_H5074_989A (String)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Paired: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                       Bonded: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Trusted: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Blocked: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                LegacyPairing: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                         RSSI: -52 (Int16)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                    Connected: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                        UUIDs:  0000180a-0000-1000-8000-00805f9b34fb 0000fef5-0000-1000-8000-00805f9b34fb 0000ec88-0000-1000-8000-00805f9b34fb
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):                      Adapter: /org/bluez/hci0 (ObjectPath)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ManufacturerData:  (DBUS_TYPE_DICT_ENTRY){ (UINT16: ec88) (DBUS_TYPE_VARIANT)[ (DBUS_TYPE_ARRAY){00ab07301c6302} ] }
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(717):             ServicesResolved: 0 (Boolean)
/home/visualstudio/projects/BlueZ-DBus/bluez-dbus.cpp(703):                 String: org.freedesktop.DBus.Properties
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
