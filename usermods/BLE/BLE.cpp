#include "BLE.h"
#include <esp_bt_device.h>

#ifdef WLED_DEBUG
void BLEUsermod::DEBUG_STATUS()
{
    DEBUG_PRINTLN("*********************************************");
    DEBUG_PRINTF_P(PSTR("WLED_CONNECTED: %d\n"), WLED_CONNECTED);
    DEBUG_PRINTF_P(PSTR("WiFi.isConnected: %d\n"), WiFi.isConnected());
    DEBUG_PRINTF_P(PSTR("apBehavior: %d\n"), apBehavior);
    DEBUG_PRINTF_P(PSTR("apActive: %d\n"), apActive);
    DEBUG_PRINTF_P(PSTR("WiFi status: %d\n"), WiFi.status());
    DEBUG_PRINTF_P(PSTR("BLE enabled: %d\n"), enabled);
    DEBUG_PRINTF_P(PSTR("BLE connected: %d\n"), SerialBLE.connected());
    DEBUG_PRINTLN("*********************************************");
    // if(initDone)
    // {
    //     NimBLEConnInfo connInfo = pServer->getPeerInfo(0);
    //     DEBUG_PRINTF_P(PSTR("BLE advertising: %d\n"), isAdvertising());
    //     DEBUG_PRINTF_P(PSTR("BLE connected devices: %d\n"), pServer->getConnectedCount());
    //     DEBUG_PRINTF_P(PSTR("OTA address %s, type %d\n"), connInfo.getAddress().toString().c_str(), connInfo.getAddress().getType());
    //     DEBUG_PRINTF_P(PSTR("ID address %s, type %d\n"), connInfo.getIdAddress().toString().c_str(), connInfo.getIdAddress().getType());
    //     DEBUG_PRINTF_P(PSTR("Bonded: %s, Authenticated: %s, Encrypted: %s, Key size: %d\n"),
    //                    connInfo.isBonded() ? F("yes") : F("no"),
    //                    connInfo.isAuthenticated() ? F("yes") : F("no"),
    //                    connInfo.isEncrypted() ? F("yes") : F("no"),
    //                    connInfo.getSecKeySize());
    // }
}
#else
void BLEUsermod::DEBUG_STATUS(){}
#endif

void BLEUsermod::setup() {
    uint8_t mac[6];
    // Read the Bluetooth MAC address
    esp_err_t status = esp_read_mac(mac, ESP_MAC_BT);
    char deviceName[19] = { 0 };
    snprintf(deviceName, sizeof(deviceName), "WLED_%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    enabled = true;
    DEBUG_PRINTF_P(PSTR("deviceName: %s\n"), deviceName);
    // serviceUUID = NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_JSON_API_SERVICE_OFFSET,
    //                          WLED_BLE_UUID_2ND_VALUE,
    //                          WLED_BLE_UUID_3RD_VALUE,
    //                          WLED_BLE_UUID_4TH_VALUE);

    DEBUG_PRINTLN(F("BLEUsermod::setup called"));
    DEBUG_STATUS();

    if(!enabled) return;
    DEBUG_PRINTLN(F("Starting NimBLE Server"));
    
    if(!initDone) {
        // SerialBLE.begin(macStr);
        SerialBLE.begin(deviceName);
        /** Initialize NimBLE and set the device name */
        // if(NimBLEDevice::init(serverDescription)) {
        //     NimBLEDevice::setSecurityAuth(true, true, true);
        //     NimBLEDevice::setSecurityIOCap(BLE_HS_IO_NO_INPUT_OUTPUT);
        //     DEBUG_PRINTLN(F("NimBLE init'd"));
        //     pServer = NimBLEDevice::createServer();
        //     if(pServer) {
        //         DEBUG_PRINTLN(F("BLE server created"));
        //         pServer->setCallbacks(this);
        //         pServer->advertiseOnDisconnect(true);

        //         pJsonApiService = pServer->createService(serviceUUID);
        //         if(pJsonApiService) {
        //             bool allCharacteristicsInitialized = true;
        //             DEBUG_PRINTF_P(PSTR("BLE %s service created\n"), F("JSON API"));

        //             // Create JSON State characteristic
        //             pStateCharacteristic =
        //                 pJsonApiService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_STATE_CHARACTERISTIC_OFFSET,
        //                                                                  WLED_BLE_UUID_2ND_VALUE,
        //                                                                  WLED_BLE_UUID_3RD_VALUE,
        //                                                                  WLED_BLE_UUID_4TH_VALUE),
        //                                                       NIMBLE_PROPERTY::READ);
        //                                                     //   | NIMBLE_PROPERTY::READ_ENC
        //                                                     //   | NIMBLE_PROPERTY::READ_AUTHEN);
        //             if(pStateCharacteristic) {
        //                 DEBUG_PRINTF_P(PSTR("BLE %s characteristic created\n"), F("state"));
        //                 pStateCharacteristic->setCallbacks(this);
        //                 updateStateCharacteristic();
        //             } else {
        //                 allCharacteristicsInitialized = false;
        //                 DEBUG_PRINTF_P(PSTR("Unable to create BLE %s characteristic\n"), F("state"));
        //             }

        //             // Create JSON Info characteristic
        //             pInfoCharacteristic =
        //                 pJsonApiService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_INFO_CHARACTERISTIC_OFFSET,
        //                                                                  WLED_BLE_UUID_2ND_VALUE,
        //                                                                  WLED_BLE_UUID_3RD_VALUE,
        //                                                                  WLED_BLE_UUID_4TH_VALUE),
        //                                                       NIMBLE_PROPERTY::READ);
        //                                                     //   | NIMBLE_PROPERTY::READ_ENC
        //                                                     //   | NIMBLE_PROPERTY::READ_AUTHEN);
        //             if(pInfoCharacteristic) {
        //                 DEBUG_PRINTF_P(PSTR("BLE %s characteristic created\n"), F("info"));
        //                 pInfoCharacteristic->setCallbacks(this);
        //                 updateInfoCharacteristic();
        //             } else {
        //                 allCharacteristicsInitialized = false;
        //                 DEBUG_PRINTF_P(PSTR("Unable to create BLE %s characteristic\n"), F("info"));
        //             }

        //             // Create JSON Effects characteristic
        //             pEffectsCharacteristic =
        //                 pJsonApiService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_EFFECTS_CHARACTERISTIC_OFFSET,
        //                                                                  WLED_BLE_UUID_2ND_VALUE,
        //                                                                  WLED_BLE_UUID_3RD_VALUE,
        //                                                                  WLED_BLE_UUID_4TH_VALUE),
        //                                                       NIMBLE_PROPERTY::READ);
        //                                                     //   | NIMBLE_PROPERTY::READ_ENC
        //                                                     //   | NIMBLE_PROPERTY::READ_AUTHEN);
        //             if(pEffectsCharacteristic) {
        //                 DEBUG_PRINTF_P(PSTR("BLE %s characteristic created\n"), F("effects"));
        //                 pEffectsCharacteristic->setCallbacks(this);
        //                 updateEffectsCharacteristic();
        //             } else {
        //                 allCharacteristicsInitialized = false;
        //                 DEBUG_PRINTF_P(PSTR("Unable to create BLE %s characteristic\n"), F("effects"));
        //             }

        //             // Create JSON Palettes characteristic
        //             pPalettesCharacteristic =
        //                 pJsonApiService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_PALETTES_CHARACTERISTIC_OFFSET,
        //                                                                  WLED_BLE_UUID_2ND_VALUE,
        //                                                                  WLED_BLE_UUID_3RD_VALUE,
        //                                                                  WLED_BLE_UUID_4TH_VALUE),
        //                                                       NIMBLE_PROPERTY::READ);
        //                                                     //   | NIMBLE_PROPERTY::READ_ENC
        //                                                     //   | NIMBLE_PROPERTY::READ_AUTHEN);
        //             if(pPalettesCharacteristic) {
        //                 DEBUG_PRINTF_P(PSTR("BLE %s characteristic created\n"), F("palettes"));
        //                 pPalettesCharacteristic->setCallbacks(this);
        //                 updatePalettesCharacteristic();
        //             } else {
        //                 allCharacteristicsInitialized = false;
        //                 DEBUG_PRINTF_P(PSTR("Unable to create BLE %s characteristic\n"), F("palettes"));
        //             }

        //             SerialBLE.begin(pJsonApiService);

        //             // if all characteristics were created successfullly, we can start the JSON API service
        //             if(allCharacteristicsInitialized) {
        //                 if(pJsonApiService->start()) {
        //                     DEBUG_PRINTF_P(PSTR("BLE %s service started\n"), F("JSON API"));
        //                     initDone = true;
        //                 } else {
        //                     DEBUG_PRINTF_P(PSTR("Unable to start BLE %s service\n"), F("JSON API"));
        //                 }
        //             }
        //         } else {
        //             DEBUG_PRINTF_P(PSTR("Unable to create BLE %s service\n"), F("JSON API"));
        //         }
        //     } else {
        //         DEBUG_PRINTLN(F("Unable to create BLE server"));
        //     }
        // } else {
        //     DEBUG_PRINTLN(F("Unable to initialize NimBLE"));
        // }
    }
    // if(initDone && !isAdvertising()) {
    //     NimBLEAdvertising* pAdvertising = pServer->getAdvertising();
    //     pAdvertising->setName("WLED");
    //     pAdvertising->addServiceUUID(serviceUUID);
    //     start();
    // }
    initDone = true;
}
  
void BLEUsermod::loop()
{
    // if usermod is disabled or called during strip updating just exit
    // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
    if (strip.isUpdating()) return;
    if(!enabled) {
        // if(isAdvertising()) stop();
        return;
    }
    if (!initDone){
        DEBUG_PRINTLN(F("BLE init not completed, calling setup from loop()"));
        setup();
    }

    if (millis() - lastTime > 2000 / portTICK_PERIOD_MS) {
        // updateStateCharacteristic();
        // updateInfoCharacteristic();
        // updateEffectsCharacteristic();
        // updatePalettesCharacteristic();
        handleSerial(SerialBLE);
        lastTime = millis();
    }
}

void BLEUsermod::connected()
{
    DEBUG_PRINTLN(F("BLEUsermod::connected() called"));
    DEBUG_STATUS();
}

void BLEUsermod::addToJsonState(JsonObject& root)
{
    DEBUG_PRINTF_P(PSTR("BLEUsermod::%s() called\n"), F("addToJsonState"));
    DEBUG_STATUS();
     if (!initDone || !enabled) return;  // prevent crash on boot applyPreset()

    JsonObject usermod = root[FPSTR(_name)];
    if (usermod.isNull()) usermod = root.createNestedObject(FPSTR(_name));

    usermod[FPSTR(_enabled)] = enabled;
}
  
void BLEUsermod::readFromJsonState(JsonObject& root) 
{
    DEBUG_PRINTF_P(PSTR("BLEUsermod::%s() called\n"), F("readFromJsonState"));
    DEBUG_STATUS();
     if (!initDone) return;  // prevent crash on boot applyPreset()
    
    JsonObject usermod = root[FPSTR(_name)];
    if (!usermod.isNull()) {
        enabled = usermod[FPSTR(_enabled)] | enabled;
    }
}

bool BLEUsermod::readFromConfig(JsonObject& root) 
{
    DEBUG_PRINTF_P(PSTR("BLEUsermod::%s() called\n"), F("readFromConfig"));
    DEBUG_STATUS();
     JsonObject top = root[FPSTR(_name)];
    bool configComplete = !top.isNull();

    configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled);

    return configComplete;
}

void BLEUsermod::addToConfig(JsonObject &root) 
{
    DEBUG_PRINTF_P(PSTR("BLEUsermod::%s() called\n"), F("addToConfig"));
    DEBUG_STATUS();
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top[FPSTR(_enabled)] = enabled;
}

// void BLEUsermod::start() {
//     if(isAdvertising() || isConnected()) return;
//     if(enabled)
//     {
//         if(!initDone)
//             setup();
//         else
//         {
//             bool whitelistOnly = (NimBLEDevice::getWhiteListCount() > 0);
//             NimBLEAdvertising* pAdvertising = NimBLEDevice::getAdvertising();
//             pAdvertising->setScanFilter(whitelistOnly, whitelistOnly);
//             DEBUG_PRINTLN(F("BLE about to pAdvertising->enableScanResponse(true)"));
//             pAdvertising->enableScanResponse(true);
//             DEBUG_PRINTLN(F("BLE starting advertising"));

//             // if(pAdvertising->start(30 * 1000))
//             if(pAdvertising->start())
//             {
//                 DEBUG_PRINTLN(F("BLE advertising started"));
//             } else {
//                 DEBUG_PRINTLN(F("Unable to start BLE advertising"));
//             }
//         }
//     }
// }

// void BLEUsermod::stop() {
//     if(!isAdvertising() && !isConnected()) return;
//     DEBUG_PRINTLN(F("stopping BLE..."));
//     std::vector<uint16_t> peers = pServer->getPeerDevices();
//     for(uint16_t handle: peers)
//     {
//         DEBUG_PRINTF_P(PSTR("Disconnecting peer %u\n"), handle);
//         pServer->disconnect(handle);
//     }
//     if(NimBLEDevice::stopAdvertising())
//         DEBUG_PRINTLN(F("BLE stopped"));
//     else
//         DEBUG_PRINTLN(F("BLE stop failed"));
// }

// NimBLEServerCallbacks overrides
//#region NimBLEServerCallbacks 
// void BLEUsermod::onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo)
// {
//     // NimBLEServerCallbacks::onConnect(pServer, connInfo);
//     DEBUG_PRINTF_P(PSTR("Client address: %s\n"), connInfo.getAddress().toString().c_str());
    
//     // // updateConnParams() to shorten te connection interval to
//     // // improve performance
//     auto minInterval = 6; // 6 * 1.25 = 7.5ms
//     auto maxInterval = minInterval;
//     auto latency_packets = 0;
//     auto timeout = 10; // 10 * 10ms = 100ms
//     // pServer->updateConnParams(connInfo.getConnHandle(),
//     //                             minInterval,
//     //                             maxInterval,
//     //                             latency_packets,
//     //                             timeout);
//     pServer->updateConnParams(connInfo.getConnHandle(), 24, 48, 6, 180);
//     // pServer->updateConnParams(connInfo.getConnHandle(), minInterval, maxInterval, latency_packets, timeout);
// }

// void BLEUsermod::onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason)
// {
//     // NimBLEServerCallbacks::onDisconnect(pServer, connInfo, reason);
//     DEBUG_PRINTLN("Client disconnected - start advertising");
//     if(!wifiEnabled) {
//         auto* pAdvertising = pServer->getAdvertising();
//         if (pAdvertising == nullptr) {
//             return;
//         }
//         pAdvertising->start();
//     }
// }

// void BLEUsermod::onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo)
// {
//     // NimBLEServerCallbacks::onMTUChange(MTU, connInfo);
//     DEBUG_PRINTF_P(PSTR("MTU updated: %u for connection ID: %u\n"), MTU, connInfo.getConnHandle());
// }

// void BLEUsermod::onAuthenticationComplete(NimBLEConnInfo& connInfo)
// {
//     // NimBLEServerCallbacks::onAuthenticationComplete(connInfo);
//     DEBUG_PRINTLN(F("onAuthenticationComplete called"));
//     /** Check that encryption was successful, if not we disconnect the client */
//     if (!connInfo.isEncrypted()) {
//         NimBLEDevice::getServer()->disconnect(connInfo.getConnHandle());
//         DEBUG_PRINTF("Encrypt connection failed - disconnecting client");
//         return;
//     }

//     DEBUG_PRINTF_P(PSTR("Secured connection to: %s\n"), connInfo.getAddress().toString().c_str());
//     if(connInfo.isBonded())
//     {
//         DEBUG_PRINTLN(F("whitelisting client"));
//         NimBLEDevice::whiteListAdd(connInfo.getAddress());
//         NimBLEDevice::whiteListAdd(connInfo.getIdAddress());
//     } else {
//         DEBUG_PRINTLN(F("client not bonded"));
//     }
// }
//#endregion

// NimBLECharacteristicCallbacks overrides
// void BLEUsermod::onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)
// {
//     // NimBLECharacteristicCallbacks::onRead(pCharacteristic, connInfo);
//     DEBUG_PRINTF_P(PSTR("%s : onRead(), value: %s\n"),
//            pCharacteristic->getUUID().toString().c_str(),
//            pCharacteristic->getValue().c_str());
// }

// void BLEUsermod::onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)
// {
//     // NimBLECharacteristicCallbacks::onWrite(pCharacteristic, connInfo);
//     DEBUG_PRINTF_P(PSTR("%s : onWrite(), value: %s\n"),
//            pCharacteristic->getUUID().toString(),
//            pCharacteristic->getValue());
// }

/**
 *  The value returned in code is the NimBLE host return code.
 */
// void BLEUsermod::onStatus(NimBLECharacteristic* pCharacteristic, int code)
// {
//     // NimBLECharacteristicCallbacks::onStatus(pCharacteristic, code);
//     DEBUG_PRINTF_P(PSTR("Notification/Indication return code: %d, %s\n"),
//                    code,
//                    NimBLEUtils::returnCodeToString(code));
// }

// /** Peer subscribed to notifications/indications */
// void BLEUsermod::onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue)
// {
//     // NimBLECharacteristicCallbacks::onSubscribe(pCharacteristic, connInfo, subValue);
//     std::string str  = "Client ID: ";
//     str             += connInfo.getConnHandle();
//     str             += " Address: ";
//     str             += connInfo.getAddress().toString();
//     if (subValue == 0) {
//         str += " Unsubscribed to ";
//     } else if (subValue == 1) {
//         str += " Subscribed to notifications for ";
//     } else if (subValue == 2) {
//         str += " Subscribed to indications for ";
//     } else if (subValue == 3) {
//         str += " Subscribed to notifications and indications for ";
//     }
//     str += std::string(pCharacteristic->getUUID());

//     DEBUG_PRINTF_P(PSTR("%s\n"), str.c_str());
// }

// DescriptorCallbacks overrides
// void BLEUsermod::onWrite(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo)
// {
//     // BLEDescriptorCallbacks::onWrite(pDescriptor, connInfo);
//     std::string dscVal = pDescriptor->getValue();
//     DEBUG_PRINTF_P(PSTR("Descriptor written value: %s"), dscVal.c_str());
// }

// void BLEUsermod::onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo)
// {
//     // BLEDescriptorCallbacks::onRead(pDescriptor, connInfo);
//     DEBUG_PRINTF_P(PSTR("%s Descriptor read"), pDescriptor->getUUID().toString().c_str());
// }

// Advertising Complete Callback
// void BLEUsermod::onAdvComplete(NimBLEAdvertising* pAdvertising)
// {
//     DEBUG_PRINTLN(F("Advertising stopped"));
//     if(pServer->getConnectedCount()) return;

//     DEBUG_PRINTLN(F("Add code here to manage automatic advertising startup..."));
// }

// void BLEUsermod::updateStateCharacteristic()
// {
//     if(!requestJSONBufferLock(23)) {
//         DEBUG_PRINTF_P(PSTR("{\"error\":%d}\n"), ERR_NOBUF);
//         return;
//     }
//     // pDoc->clear();
//     // JsonObject state = pDoc->createNestedObject(F("state"));
//     // serializeState(state);
//     // serializeJson(*pDoc, _stateCharacteristicBuffer);
//     // pStateCharacteristic->setValue(_stateCharacteristicBuffer);
//     std::string v = "state";
//     pStateCharacteristic->setValue(v);
//     releaseJSONBufferLock();
// }

// void BLEUsermod::updateInfoCharacteristic()
// {
//     if(!requestJSONBufferLock(24)) {
//         DEBUG_PRINTF_P(PSTR("{\"error\":%d}\n"), ERR_NOBUF);
//         return;
//     }
//     // pDoc->clear();
//     // JsonObject info = pDoc->createNestedObject(F("info"));
//     // serializeInfo(info);
//     // serializeJson(*pDoc, _infoCharacteristicBuffer);
//     // pStateCharacteristic->setValue(_infoCharacteristicBuffer);
//     std::string v = "info";
//     pInfoCharacteristic->setValue(v);
//     releaseJSONBufferLock();
// }

// void BLEUsermod::updateEffectsCharacteristic()
// {
//     if(!requestJSONBufferLock(25)) {
//         DEBUG_PRINTF_P(PSTR("{\"error\":%d}\n"), ERR_NOBUF);
//         return;
//     }
//     // pDoc->clear();
//     // JsonArray effects = pDoc->createNestedArray(F("effects"));
//     // serializeModeNames(effects);
//     // serializeJson(*pDoc, _effectsCharacteristicBuffer);
//     // pEffectsCharacteristic->setValue(_effectsCharacteristicBuffer);
//     std::string v = "effects";
//     pEffectsCharacteristic->setValue(v);
//     releaseJSONBufferLock();
// }

// void BLEUsermod::updatePalettesCharacteristic()
// {
//     if(!requestJSONBufferLock(26)) {
//         DEBUG_PRINTF_P(PSTR("{\"error\":%d}\n"), ERR_NOBUF);
//         return;
//     }
//     pDoc->clear();
//     (*pDoc)[F("palettes")] = serialized((const __FlashStringHelper*)JSON_palette_names);
//     serializeJson(*pDoc, _palettesCharacteristicBuffer);
//     pPalettesCharacteristic->setValue(_palettesCharacteristicBuffer);
//     releaseJSONBufferLock();
// }

const char BLEUsermod::_name[]       PROGMEM = "BLE";
const char BLEUsermod::_enabled[]    PROGMEM = "enabled";

static BLEUsermod usermod_BLE;
REGISTER_USERMOD(usermod_BLE);
