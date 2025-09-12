#ifdef USERMOD_BLE
#ifndef USERMOD_BLE_H
#define USERMOD_BLE_H

#include <NimBLEDevice.h>
#include <queue>
#include "wled.h"

const uint32_t WLED_BLE_UUID_1ST_VALUE = 0x6E400000UL;
const uint16_t WLED_BLE_UUID_2ND_VALUE = 0xB5A3U; 
const uint16_t WLED_BLE_UUID_3RD_VALUE = 0xF393U;
const uint64_t WLED_BLE_UUID_4TH_VALUE = 0xE0A9E50E24DCCA9ELLU;
#define WLED_BLE_SERVICE_OFFSET 1
#define WLED_BLE_RX_OFFSET      2
#define WLED_BLE_TX_OFFSET      3

class BLEUsermodServerCallbacks: public NimBLEServerCallbacks
{
    public:
    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override;
};

class BLEUsermodCharacteristicCallbacks: public NimBLECharacteristicCallbacks
{
    private:
    std::queue<uint8_t>* rxBuffer = nullptr;

    public:
    explicit BLEUsermodCharacteristicCallbacks(std::queue<uint8_t>* buffer) : rxBuffer(buffer) {}
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override;
};

class BLEUsermod : public Usermod,
                   public Stream
{
    friend class BLEUsermodCharacteristicCallbacks;

    private:
    static const char _name[];
    static const char _enabled[];

    char                  deviceName[23]      = { 0 };
    bool                  enabled             = false;
    bool                  initDone            = false;
    unsigned long         lastTime            = 0;

    std::queue<uint8_t> receiveBuffer;

    NimBLEServer* pServer = nullptr;
    NimBLEService* pService = nullptr;
    NimBLECharacteristic* pRxCharacteristic = nullptr;
    NimBLECharacteristic* pTxCharacteristic = nullptr;
  
    public:
    BLEUsermod();
    void DEBUG_STATUS();
    // ******** Usermod overrides *******
    void setup() override;
    void loop() override;
    void addToJsonState(JsonObject& root) override;
    void readFromJsonState(JsonObject& root) override;
    bool readFromConfig(JsonObject& root) override;
    void addToConfig(JsonObject &root) override;
    uint16_t getId() override { return USERMOD_ID_BLE; }

    // ******** Stream overrides *******
    BLEUsermod(BLEUsermod const& other) = delete;      // disable copy constructor
    void operator=(BLEUsermod const& other) = delete; // disable assign constructor
    inline int available() override { return receiveBuffer.size(); }
    inline int peek() override { return receiveBuffer.front(); }
    int read() override;
    size_t write(const uint8_t* buffer, size_t bufferSize) override;
    size_t write(uint8_t byte) override;
    void flush(void) override { this->pTxCharacteristic->notify(true); }
    // size_t println(void) { Stream::write("\n"); flush(); return 1; }

    void enable(bool enable) { enabled = enable; }
    bool isEnabled() { return enabled; }
    bool isAdvertising() { return NimBLEDevice::getAdvertising()->isAdvertising(); }
    bool isConnected() { return pServer != nullptr && pServer->getConnectedCount() > 0; }
    auto getRxCharacteristic() -> NimBLECharacteristic* { return pRxCharacteristic; }
    auto getTxCharacteristic() -> NimBLECharacteristic* { return pTxCharacteristic; }
    void begin(char* deviceName);
    void end();
};
#endif
#endif
