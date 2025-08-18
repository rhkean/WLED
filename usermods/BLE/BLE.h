#ifdef USERMOD_BLE
#ifndef USERMOD_BLE_H
#define USERMOD_BLE_H

#define WLED_BLE_SERVICE_UUID        "01FA0001-46C9-4507-84BB-F2BE3F24C47A"
#define WLED_BLE_STATE_CHARACTERISTIC_UUID "01FA0002-46C9-4507-84BB-F2BE3F24C47A"
#include "wled.h"
#include "NimBLEDevice.h"
#include <vector>

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
    NimBLEServer*         pServer               = nullptr;
    NimBLEService*        pService              = nullptr;
    NimBLECharacteristic* pStateCharacteristic  = nullptr;
 
    std::vector<char> _stateCharacteristicBuffer;
    
    //void shutdownWiFi();
    void DEBUG_STATUS();

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
