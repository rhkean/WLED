# Usermods API v2 example usermod

In this usermod file you can find the documentation on how to take advantage of the new version 2 usermods!

## Installation 

Copy `usermod_v2_example.h` to the wled00 directory.  
Uncomment the corresponding lines in `usermods_list.cpp` and compile!  
_(You shouldn't need to actually install this, it does nothing useful)_
```
CONFIG_BT_ENABLED=y
CONFIG_BT_NIMBLE_ENABLED=y
CONFIG_BT_NIMBLE_NVS_PERSIST=y
CONFIG_OPENTHREAD_RX_ON_WHEN_IDLE=y
CONFIG_NIMBLE_CPP_LOG_LEVEL_DEBUG=y
CONFIG_NIMBLE_CPP_ENABLE_RETURN_CODE_TEXT=y
CONFIG_NIMBLE_CPP_ENABLE_GAP_EVENT_CODE_TEXT=y
CONFIG_NIMBLE_CPP_ENABLE_ADVERTISEMENT_TYPE_TEXT=y

NimBLECharacteristic*   pCharacteristic         = nullptr;
bool                    deviceConnected         = false;
bool                    oldDeviceConnected      = false;
uint32_t                value                   = 0;
#define SERVICE_UUID        "00000000-0000-0000-0000-000000000000"
#define CHARACTERISTIC_UUID "00000000-0000-0000-0000-000000000001"

class ServerCallbacks : public NimBLEServerCallbacks {
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
        deviceConnected = true;
    }
    voic onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
        // Peer disconnected, add them to the whitelist
        // This allows us to use the whitelist to filter connection attempts
        // which will minimize reconnection time.
        NimBLEDevice::whitelistAdd(connInfo.getAddress());
        deviceConnected = false;
    }
} serverCallbacks;
void onAdvComplete(NimBLEAdvertising* pAdvertising) {
    DEBUG_PRINTLN(F("Advertising stopped"));
    if(deviceConnected) {
        return;
    }
    // if advertising timed out without connection, start advertising without whitelist filter
    pAdvertising->setScanFilter(false, false);
    pAdvertising->start();
}
setup(){
    NimBLEDevice::init(serverDescription)
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(&serverCallbacks)
    pServer->advertiseOnDisconnect(false)
    NimBLEService* pService = pServer->createService(SERVICE_UUID)
    NimBLECharacteristic* pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID
                                                                          , NimBLE_PROPERTY::READ
                                                                          | NimBLE_PROPERTY::WRITE
                                                                          | NimBLE_PROPERTY::NOTIFY);
    pService->start()

    NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID)
    pAdvertising->enableScanResponse(false);
    pAdvertising->setAdvertisingCompleteCallback(onAdvComplete);
    pAdvertising->start();
}
loop() {
    if(deviceConnected) {
        pCharacteristic->setValue((uint8_t)&value, 4);
        pCharacteristic->notify()
        value++;
    }
    
    if(!deviceConnected && oldDeviceConnected) {
        NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
        if(NimBLEDevice::getWhiteListCount() > 0) {
            // allow anyone to scan but only whitelisted can connect.
            pAdvertising->setScanFilter(false, true);
        }
        // advertise with whitelist for 30 seconds
        pAdvertising->start(30 * 1000);
        DEBUG_PRINTLN(F("start advertising"));
        oldDeviceConnected = deviceConnected;
    }
    if(deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
    }
    delay(2000);
}
```