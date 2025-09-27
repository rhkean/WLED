# BLE

Adds BluetoothLE connectivity to WLED

This will require a bluetooth capable version of WLED-Native-Android ( I'm working on it :-)  )

## Installation 

Add 'BLE' to 'custom_usermods' in your platformio_override.ini.

Example:
```
[env:BLE]
extends = env:esp32dev
custom_usermods = ${env:esp32dev.custom_usermods} BLE
```