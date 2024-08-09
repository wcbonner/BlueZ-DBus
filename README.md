# BlueZ-DBus

This program is an attempt to learn recieving Bluetooth LE Advertisment recieving using the libdbus-1-dev library 

https://dbus.freedesktop.org/doc/api/html/index.html

I started with a code example I found at https://forums.raspberrypi.com/viewtopic.php?t=353889&sid=2b10dcb0eca390f135ee8c64da253a9c and heavily modified to my own style.

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

