#ifdef USERMOD_BLE
#include "BLE.h"
#include <esp_bt_device.h>
#include <HTTPClient.h>

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
   uint8_t mac[6];
   if(esp_read_mac(mac, ESP_MAC_BT) == ESP_OK) {
        snprintf(deviceName, sizeof(deviceName), "WLED_%02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    } else {
        snprintf(deviceName, sizeof(deviceName), "%s", serverDescription);
    }
    DEBUG_PRINTF_P(PSTR("deviceName: %s\n"), deviceName);
}

void BLEUsermod::setup() {
    DEBUG_STATUS();
    if(!enabled) return;    
    if(!initDone) begin();
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
        handleJson();
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
    pJsonTxCharacteristic->setValue(const_cast<uint8_t*>(buffer), bufferSize);
    return bufferSize;
}

size_t BLEUsermod::write(uint8_t byte) {
    if (!isConnected()) return 0;
    DEBUG_PRINTLN(F("short write called"));
    pJsonTxCharacteristic->setValue(&byte, 1);
    return 1;
}

void BLEUsermod::begin() {
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

    pJsonRxCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_JSON_RX_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::WRITE
                                       | NIMBLE_PROPERTY::WRITE_NR,
                                       4096);

    pJsonTxCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_JSON_TX_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ
                                       | NIMBLE_PROPERTY::NOTIFY,
                                       4096);
    pJsonRxCharacteristic->setCallbacks(new BLEUsermodCharacteristicCallbacks(&receiveBuffer));

#ifndef WLED_DISABLE_2D 
    pHttpLiveview2DCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_LIVEVIEW2D_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_liveviewws2D_length);
    pHttpLiveview2DCharacteristic->setValue((uint8_t*)PAGE_liveviewws2D, PAGE_liveviewws2D_length);
    pHttpLiveview2DCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED 2D Live View");
#endif

    pHttpLiveviewCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_LIVEVIEW_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_liveview_length);
    pHttpLiveviewCharacteristic->setValue((uint8_t*)PAGE_liveview, PAGE_liveview_length);
    pHttpLiveviewCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Live View");

    pHttpCommonJsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_COMMONJS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       JS_common_length);
    pHttpCommonJsCharacteristic->setValue((uint8_t*)JS_common, JS_common_length);
    pHttpCommonJsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Common JS");

    pHttpStyleCssCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_STYLECSS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settingsCss_length);
    pHttpStyleCssCharacteristic->setValue((uint8_t*)PAGE_settingsCss, PAGE_settingsCss_length);
    pHttpStyleCssCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Settings CSS");

    pHttpFaviconCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_FAVICON_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       favicon_length);
    pHttpFaviconCharacteristic->setValue((uint8_t*)favicon, favicon_length);
    pHttpFaviconCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Favicon");

    pHttpIndexCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_INDEX_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_index_length);
    pHttpIndexCharacteristic->setValue((uint8_t*)PAGE_index, PAGE_index_length);
    pHttpIndexCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Main");

#ifdef WLED_ENABLE_PIXART
    pHttpPixartCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_PIXART_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_pixart_length);
    pHttpPixartCharacteristic->setValue((uint8_t*)PAGE_pixart, PAGE_pixart_length);
    pHttpPixartCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED PixArt");
#endif
#ifndef WLED_DISABLE_PXMAGIC
    pHttpPxmagicCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_PXMAGIC_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_pxmagic_length);
    pHttpPxmagicCharacteristic->setValue((uint8_t*)PAGE_pxmagic, PAGE_pxmagic_length);
    pHttpPxmagicCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED PxMagic");
#endif
    pHttpCpalCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_CPAL_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_cpal_length);
    pHttpCpalCharacteristic->setValue((uint8_t*)PAGE_cpal, PAGE_cpal_length);
    pHttpCpalCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Color Palettes");

    pHttpWifiSettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_WIFI_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settings_wifi_length);
    pHttpWifiSettingsCharacteristic->setValue((uint8_t*)PAGE_settings_wifi, PAGE_settings_wifi_length);
    pHttpWifiSettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED WiFi Settings");

    pHttpLedsSettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_LEDS_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settings_leds_length);
    pHttpLedsSettingsCharacteristic->setValue((uint8_t*)PAGE_settings_leds, PAGE_settings_leds_length);
    pHttpLedsSettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED LED Settings");

    pHttpUiSettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_UI_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settings_ui_length);
    pHttpUiSettingsCharacteristic->setValue((uint8_t*)PAGE_settings_ui, PAGE_settings_ui_length);
    pHttpUiSettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED UI Settings");

    pHttpSyncSettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_SYNC_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settings_sync_length);
    pHttpSyncSettingsCharacteristic->setValue((uint8_t*)PAGE_settings_sync, PAGE_settings_sync_length);
    pHttpSyncSettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Sync Settings");

    pHttpTimeSettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_TIME_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settings_time_length);
    pHttpTimeSettingsCharacteristic->setValue((uint8_t*)PAGE_settings_time, PAGE_settings_time_length);
    pHttpTimeSettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Time Settings");

    pHttpSecuritySettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_SECURITY_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settings_sec_length);
    pHttpSecuritySettingsCharacteristic->setValue((uint8_t*)PAGE_settings_sec, PAGE_settings_sec_length);
    pHttpSecuritySettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Security Settings");

    pHttpDmxSettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_DMX_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settings_dmx_length);
    pHttpDmxSettingsCharacteristic->setValue((uint8_t*)PAGE_settings_dmx, PAGE_settings_dmx_length);
    pHttpDmxSettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED DMX Settings");

    pHttpUmodsSettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_UMODS_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settings_um_length);
    pHttpUmodsSettingsCharacteristic->setValue((uint8_t*)PAGE_settings_um, PAGE_settings_um_length);
    pHttpUmodsSettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Usermod Settings");

    pHttpUpdateCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_UPDATE_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_update_length);
    pHttpUpdateCharacteristic->setValue((uint8_t*)PAGE_update, PAGE_update_length);
    pHttpUpdateCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Update Page");

    pHttp2DSettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_2D_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settings_2D_length);
    pHttp2DSettingsCharacteristic->setValue((uint8_t*)PAGE_settings_2D, PAGE_settings_2D_length);
    pHttp2DSettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED 2D Settings");

    pHttpPinSettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_PIN_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settings_pin_length);
    pHttpPinSettingsCharacteristic->setValue((uint8_t*)PAGE_settings_pin, PAGE_settings_pin_length);
    pHttpPinSettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Pin Settings");

    pHttpCssSettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_CSS_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settingsCss_length);
    pHttpCssSettingsCharacteristic->setValue((uint8_t*)PAGE_settingsCss, PAGE_settingsCss_length);
    pHttpCssSettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Settings CSS");

    pHttpWelcomeCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_WELCOME_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_welcome_length);
    pHttpWelcomeCharacteristic->setValue((uint8_t*)PAGE_welcome, PAGE_welcome_length);
    pHttpWelcomeCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Welcome Page");

    pHttpSettingsCharacteristic =
        pService->createCharacteristic(NimBLEUUID(WLED_BLE_UUID_1ST_VALUE + WLED_BLE_HTTP_SETTINGS_OFFSET,
                                                  WLED_BLE_UUID_2ND_VALUE,
                                                  WLED_BLE_UUID_3RD_VALUE,
                                                  WLED_BLE_UUID_4TH_VALUE),
                                       NIMBLE_PROPERTY::READ,
                                       PAGE_settings_length);
    pHttpSettingsCharacteristic->setValue((uint8_t*)PAGE_settings, PAGE_settings_length);
    pHttpSettingsCharacteristic->createDescriptor("2901", NIMBLE_PROPERTY::READ, 23)->setValue("WLED Settings Main");
    

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

void BLEUsermod::handleJson() {
    if(available() > 0) {
        bool verboseResponse = false;
        if (!requestJSONBufferLock(16)) {
            printf_P(PSTR("{\"error\":%d}\n"), ERR_NOBUF);
            return;
        }
        DeserializationError error = deserializeJson(*pDoc, *this);
        if (!error) {
            pDoc->clear();
            verboseResponse = deserializeState(pDoc->as<JsonObject>());
            //only send response if TX pin is unused for other purposes
            if (verboseResponse) {
                pDoc->clear();
                JsonObject stateDoc = pDoc->createNestedObject("state");
                serializeState(stateDoc);
                JsonObject info  = pDoc->createNestedObject("info");
                serializeInfo(info);

                serializeJson(*pDoc, *this);
                flush();
            }
            printf("WLED %d", VERSION);
        }
        releaseJSONBufferLock();
    }
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
    auto rxValue = pCharacteristic->getValue();
    for (int i = 0; i < rxValue.length(); i++) {
        rxBuffer->push(rxValue[i]);
    }
}
#endif
