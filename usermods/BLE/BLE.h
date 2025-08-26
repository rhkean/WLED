#ifdef USERMOD_BLE
#ifndef USERMOD_BLE_H
#define USERMOD_BLE_H

#define BLESERIAL_USE_NIMBLE true

#define WLED_BLE_JSON_API_SERVICE_OFFSET        1   // "01FA0001-46C9-4507-84BB-F2BE3F24C47A"
#define WLED_BLE_STATE_CHARACTERISTIC_OFFSET    2   // "01FA0002-46C9-4507-84BB-F2BE3F24C47A"
#define WLED_BLE_INFO_CHARACTERISTIC_OFFSET     3   // "01FA0003-46C9-4507-84BB-F2BE3F24C47A"
#define WLED_BLE_EFFECTS_CHARACTERISTIC_OFFSET  4   // "01FA0004-46C9-4507-84BB-F2BE3F24C47A"
#define WLED_BLE_PALETTES_CHARACTERISTIC_OFFSET 5   // "01FA0005-46C9-4507-84BB-F2BE3F24C47A"

#include "wled.h"
#include <BLESerial.h>
#include <vector>

const uint32_t WLED_BLE_UUID_1ST_VALUE = 0x01FA0000UL;
const uint16_t WLED_BLE_UUID_2ND_VALUE = 0x46C9U; 
const uint16_t WLED_BLE_UUID_3RD_VALUE = 0x4507U;
//const uint64_t WLED_BLE_UUID_4TH_VALUE = 0x84BBF2BE3F24C47ALLU;
#define WLED_BLE_UUID_4TH_VALUE 0x84BBF2BE3F24C47ALLU

class BLEUsermod : public Usermod
                        , NimBLEServerCallbacks
                        , NimBLECharacteristicCallbacks
                        , NimBLEDescriptorCallbacks
{
    private:

    bool                  deviceConnected     = false;
    bool                  oldDeviceConnected  = false;
    bool                  enabled             = false;
    bool                  initDone            = false;
    unsigned long         lastTime            = 0;
  
    static const char _name[];
    static const char _enabled[];
  
    // Private class members. You can declare variables and functions only accessible to your usermod here
    NimBLEServer*         pServer                   = nullptr;
    NimBLEService*        pJsonApiService           = nullptr;
    NimBLECharacteristic* pStateCharacteristic      = nullptr;
    NimBLECharacteristic* pInfoCharacteristic       = nullptr;
    NimBLECharacteristic* pEffectsCharacteristic    = nullptr;
    NimBLECharacteristic* pPalettesCharacteristic   = nullptr;
    NimBLEUUID serviceUUID;
 
    std::string _stateCharacteristicBuffer;
    std::string _infoCharacteristicBuffer;
    std::string _effectsCharacteristicBuffer;
    std::string _palettesCharacteristicBuffer;

    BLESerial<> SerialBLE;
    
    //void shutdownWiFi();
    void DEBUG_STATUS();
    void updateStateCharacteristic();
    void updateInfoCharacteristic();
    void updateEffectsCharacteristic();
    void updatePalettesCharacteristic();

    public:
    void setup() override;
    void loop() override;
    void connected() override;
    void addToJsonState(JsonObject& root) override;
    void readFromJsonState(JsonObject& root) override;
    bool readFromConfig(JsonObject& root) override;
    void addToConfig(JsonObject &root) override;
    //bool handleButton(uint8_t b) override;
    uint16_t getId() override {return USERMOD_ID_BLE;}
    void start();
    void stop();
    void enable(bool enable) { enabled = enable; }
    bool isEnabled() {return enabled; }
    bool isAdvertising(){return NimBLEDevice::getAdvertising()->isAdvertising();}
    uint8_t isConnected(){return pServer->getConnectedCount();}

    // NimBLEServer Callbacks
    void onAuthenticationComplete(NimBLEConnInfo& connInfo) override;
    void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override;
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
    void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override;

    // NimBLECharacteristic Callbacks
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
    void onStatus(NimBLECharacteristic* pCharacteristic, int code) override;
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override;

    // NimBLEDescriptor Callbacks
    void onWrite(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override;
    void onRead(NimBLEDescriptor* pDescriptor, NimBLEConnInfo& connInfo) override;
    void onAdvComplete(NimBLEAdvertising* pAdvertising);
};
#endif
#endif
