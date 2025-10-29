#ifdef USERMOD_BLE
#ifndef USERMOD_BLE_H
#define USERMOD_BLE_H

#include <NimBLEDevice.h>
#include <queue>
#include "wled.h"
#include "html.h"

//"01FA0001-46C9-4507-84BB-F2BE3F24C47A"
const uint32_t WLED_BLE_UUID_1ST_VALUE = 0x01FA0001UL;
const uint16_t WLED_BLE_UUID_2ND_VALUE = 0x46C9U; 
const uint16_t WLED_BLE_UUID_3RD_VALUE = 0x4507U;
const uint64_t WLED_BLE_UUID_4TH_VALUE = 0x84BBF2BE3F24C47ALLU;
#define WLED_BLE_SERVICE_OFFSET                  1
#define WLED_BLE_JSON_RX_OFFSET                  2
#define WLED_BLE_JSON_TX_OFFSET                  3
#define WLED_BLE_HTTP_LIVEVIEW2D_OFFSET          4
#define WLED_BLE_HTTP_LIVEVIEW_OFFSET            5
#define WLED_BLE_HTTP_COMMONJS_OFFSET            6
#define WLED_BLE_HTTP_STYLECSS_OFFSET            7
#define WLED_BLE_HTTP_FAVICON_OFFSET             8
#define WLED_BLE_HTTP_INDEX_OFFSET               9
#ifdef WLED_ENABLE_PIXART
#define WLED_BLE_HTTP_PIXART_OFFSET             10
#endif
#ifndef WLED_DISABLE_PXMAGIC
#define WLED_BLE_HTTP_PXMAGIC_OFFSET            11
#endif
#define WLED_BLE_HTTP_CPAL_OFFSET               12
#define WLED_BLE_HTTP_WIFI_SETTINGS_OFFSET      13
#define WLED_BLE_HTTP_LEDS_SETTINGS_OFFSET      14
#define WLED_BLE_HTTP_UI_SETTINGS_OFFSET        15
#define WLED_BLE_HTTP_SYNC_SETTINGS_OFFSET      16
#define WLED_BLE_HTTP_TIME_SETTINGS_OFFSET      17
#define WLED_BLE_HTTP_SECURITY_SETTINGS_OFFSET  18
#define WLED_BLE_HTTP_DMX_SETTINGS_OFFSET       19
#define WLED_BLE_HTTP_UMODS_SETTINGS_OFFSET     20
#define WLED_BLE_HTTP_UPDATE_OFFSET             21
#define WLED_BLE_HTTP_2D_SETTINGS_OFFSET        22
#define WLED_BLE_HTTP_PIN_SETTINGS_OFFSET       23
#define WLED_BLE_HTTP_CSS_SETTINGS_OFFSET       24
#define WLED_BLE_HTTP_WELCOME_OFFSET            25
#define WLED_BLE_HTTP_SETTINGS_OFFSET           26

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

    char            deviceName[23]  = { 0 };
    bool            enabled         = false;
    bool            initDone        = false;
    unsigned long   lastTime        = 0;

    std::queue<uint8_t> receiveBuffer;

    NimBLEServer* pServer = nullptr;
    NimBLEService* pService = nullptr;
    NimBLECharacteristic* pJsonRxCharacteristic                 = nullptr;
    NimBLECharacteristic* pJsonTxCharacteristic                 = nullptr;
#ifndef WLED_DISABLE_2D
    NimBLECharacteristic* pHttpLiveview2DCharacteristic         = nullptr;
#endif
    NimBLECharacteristic* pHttpLiveviewCharacteristic           = nullptr;
    NimBLECharacteristic* pHttpCommonJsCharacteristic           = nullptr;
    NimBLECharacteristic* pHttpStyleCssCharacteristic           = nullptr;
    NimBLECharacteristic* pHttpFaviconCharacteristic            = nullptr;
    NimBLECharacteristic* pHttpIndexCharacteristic              = nullptr;
#ifdef WLED_ENABLE_PIXART
    NimBLECharacteristic* pHttpPixartCharacteristic             = nullptr;
#endif
#ifndef WLED_DISABLE_PXMAGIC
    NimBLECharacteristic* pHttpPxmagicCharacteristic            = nullptr;
#endif
    NimBLECharacteristic* pHttpCpalCharacteristic               = nullptr;
    NimBLECharacteristic* pHttpWifiSettingsCharacteristic       = nullptr;
    NimBLECharacteristic* pHttpLedsSettingsCharacteristic       = nullptr;
    NimBLECharacteristic* pHttpUiSettingsCharacteristic         = nullptr;
    NimBLECharacteristic* pHttpSyncSettingsCharacteristic       = nullptr;
    NimBLECharacteristic* pHttpTimeSettingsCharacteristic       = nullptr;
    NimBLECharacteristic* pHttpSecuritySettingsCharacteristic   = nullptr;
    NimBLECharacteristic* pHttpDmxSettingsCharacteristic        = nullptr;
    NimBLECharacteristic* pHttpUmodsSettingsCharacteristic      = nullptr;
    NimBLECharacteristic* pHttpUpdateCharacteristic             = nullptr;
    NimBLECharacteristic* pHttp2DSettingsCharacteristic         = nullptr;
    NimBLECharacteristic* pHttpPinSettingsCharacteristic        = nullptr;
    NimBLECharacteristic* pHttpCssSettingsCharacteristic        = nullptr;
    NimBLECharacteristic* pHttpWelcomeCharacteristic            = nullptr;
    NimBLECharacteristic* pHttpSettingsCharacteristic           = nullptr;
  
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
    void flush(void) override { this->pJsonTxCharacteristic->notify(true); }
    // size_t println(void) { Stream::write("\n"); flush(); return 1; }

    void handleJson();
    void enable(bool enable) { enabled = enable; }
    bool isEnabled() { return enabled; }
    bool isAdvertising() { return NimBLEDevice::getAdvertising()->isAdvertising(); }
    bool isConnected() { return pServer != nullptr && pServer->getConnectedCount() > 0; }
    auto getRxCharacteristic() -> NimBLECharacteristic* { return pJsonRxCharacteristic; }
    auto getTxCharacteristic() -> NimBLECharacteristic* { return pJsonTxCharacteristic; }
    void begin();
    void end();
};
#endif
#endif
