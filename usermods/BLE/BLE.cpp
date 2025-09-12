#include "BLE.h"
#include <esp_bt_device.h>

const char BLEUsermod::_name[]       PROGMEM = "BLE";
const char BLEUsermod::_enabled[]    PROGMEM = "enabled";

static BLEUsermod usermod_BLE;
REGISTER_USERMOD(usermod_BLE);

#ifdef WLED_DEBUG
void BLEUsermod::DEBUG_STATUS()
{
    DEBUG_PRINTLN(F("************* BLE DEBUG *************"));
    DEBUG_PRINTF_P(PSTR("BLE enabled: %d\n"), enabled);
    DEBUG_PRINTF_P(PSTR("BLE advertising: %d\n"), isAdvertising());
    DEBUG_PRINTF_P(PSTR("BLE connected: %d\n"), isConnected());
    if(initDone && isConnected())
    {
        NimBLEConnInfo connInfo = pServer->getPeerInfo(0);
        DEBUG_PRINTF_P(PSTR("BLE connected devices: %d\n"), pServer->getConnectedCount());
        DEBUG_PRINTF_P(PSTR("OTA address %s, type %d\n"), connInfo.getAddress().toString().c_str(), connInfo.getAddress().getType());
        DEBUG_PRINTF_P(PSTR("ID address %s, type %d\n"), connInfo.getIdAddress().toString().c_str(), connInfo.getIdAddress().getType());
        DEBUG_PRINTF_P(PSTR("Bonded: %s, Authenticated: %s, Encrypted: %s, Key size: %d\n"),
                       connInfo.isBonded() ? F("yes") : F("no"),
                       connInfo.isAuthenticated() ? F("yes") : F("no"),
                       connInfo.isEncrypted() ? F("yes") : F("no"),
                       connInfo.getSecKeySize());
    }
    DEBUG_PRINTLN(F("************* BLE DEBUG *************"));
}
#else
void BLEUsermod::DEBUG_STATUS(){}
#endif

BLEUsermod::BLEUsermod() {
#if !defined(USERMOD_BLE_FORCE_SERVERNAME) || !USERMOD_BLE_FORCE_SERVERNAME  
   uint8_t mac[6];
   if(esp_read_mac(mac, ESP_MAC_BT) == ESP_OK) {
        snprintf(deviceName, sizeof(deviceName), "WLED_%02X%02X%02X%02X%02X%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
        DEBUG_PRINTF_P(PSTR("deviceName: %s\n"), deviceName);
    } else 
#endif
    {
        snprintf(deviceName, sizeof(deviceName), "%s", SERVERNAME);
    }
}

void BLEUsermod::setup() {
    DEBUG_STATUS();
    if(!enabled) return;    
    if(!initDone) begin(deviceName);
    initDone = true;
}
  
void BLEUsermod::loop()
{
    // if usermod is disabled or called during strip updating just exit
    // NOTE: on very long strips strip.isUpdating() may always return true so update accordingly
    if (strip.isUpdating()) return;
    if(!enabled) {
        end();
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
        handleSerial(*this);
        lastTime = millis();
    }
}

void BLEUsermod::addToJsonState(JsonObject& root)
{
     if (!initDone || !enabled) return;  // prevent crash on boot applyPreset()

    JsonObject usermod = root[FPSTR(_name)];
    if (usermod.isNull()) usermod = root.createNestedObject(FPSTR(_name));

    usermod[FPSTR(_enabled)] = enabled;
}
  
void BLEUsermod::readFromJsonState(JsonObject& root) 
{
     if (!initDone) return;  // prevent crash on boot applyPreset()
    
    JsonObject usermod = root[FPSTR(_name)];
    if (!usermod.isNull()) {
        enabled = usermod[FPSTR(_enabled)] | enabled;
    }
}

bool BLEUsermod::readFromConfig(JsonObject& root) 
{
    JsonObject top = root[FPSTR(_name)];
    bool configComplete = !top.isNull();

    configComplete &= getJsonValue(top[FPSTR(_enabled)], enabled);

    return configComplete;
}

void BLEUsermod::addToConfig(JsonObject &root) 
{
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top[FPSTR(_enabled)] = enabled;
}

int BLEUsermod::read() {
    auto front = receiveBuffer.front();
    receiveBuffer.pop();
    return front;
}

size_t BLEUsermod::write(const uint8_t* buffer, size_t bufferSize){
    if (!isConnected()) return 0;
    DEBUG_PRINTLN(F("long write called"));
    pTxCharacteristic->setValue(const_cast<uint8_t*>(buffer), bufferSize);
    return bufferSize;
}

size_t BLEUsermod::write(uint8_t byte) {
    if (!isConnected()) return 0;
    DEBUG_PRINTLN(F("short write called"));
    pTxCharacteristic->setValue(&byte, 1);
    return 1;
}

void BLEUsermod::begin(char* deviceName) {
    DEBUG_PRINTF_P(PSTR("Initializing BLE device with name '%s'"), deviceName);
    NimBLEDevice::init(deviceName);
    pServer = NimBLEDevice::createServer();
    if(!pServer) return;
    pServer->setCallbacks(new BLEUsermodServerCallbacks());

    pService = pServer->createService(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_SERVICE_OFFSET,
                                                 WLED_BLE_UUID_2ND_VALUE,
                                                 WLED_BLE_UUID_3RD_VALUE,
                                                 WLED_BLE_UUID_4TH_VALUE));
    if(!pService) return;

    pRxCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_RX_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::WRITE
                                       | NIMBLE_PROPERTY::WRITE_NR,
                                       4096);

    pTxCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_TX_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ
                                       | NIMBLE_PROPERTY::NOTIFY,
                                       4096);

    pRxCharacteristic->setCallbacks(new BLEUsermodCharacteristicCallbacks(&receiveBuffer));

    if(!pService->start()) return;

    NimBLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->setName(deviceName);
    if(!pAdvertising->start()) return;
}

void BLEUsermod::end()
{
    if(NimBLEDevice::isInitialized())
    {
        std::vector<uint16_t> connectedHandles = pServer->getPeerDevices();
        for (uint16_t connHandle : connectedHandles) {
            pServer->disconnect(connHandle, BLE_ERR_REM_USER_CONN_TERM); // Disconnect with a reason
        }
        pServer->getAdvertising()->stop();
        NimBLEDevice::deinit(true);
    }
    initDone = false;
}

void BLEUsermodServerCallbacks::
    onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason)
{
    auto* pAdvertising = pServer->getAdvertising();
    if (pAdvertising == nullptr) {
        return;
    }
    pAdvertising->start();
}

void BLEUsermodCharacteristicCallbacks::
    onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo)
{
    DEBUG_PRINTLN(F("BLE Characteristic onWrite called"));
    auto rxValue = pCharacteristic->getValue();
    for (int i = 0; i < rxValue.length(); i++) {
        DEBUG_PRINTF_P(PSTR("%c,"), rxValue[i]);
        rxBuffer->push(rxValue[i]);
    }
}
