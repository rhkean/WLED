#ifndef BLESERIAL_H
#define BLESERIAL_H

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <queue>

template<typename T>
class BLESerialCharacteristicCallbacks;

template<typename T>
class BLESerialServerCallbacks;

/**
 * @tparam T Type of the receive buffer
 */
template<typename T = std::queue<uint8_t>>
class BLESerial : public Stream {
    friend class BLESerialCharacteristicCallbacks<T>;
    friend class BLESerialServerCallbacks<T>;

  private:
    T m_receiveBuffer;

    /**
     * BLE server instance
     * @note This is only used if the BLESerial instance is managing the BLE server
     */
    NimBLEServer* m_pServer = nullptr;

    /**
     * BLE service instance
     * @note This is only used if the BLESerial instance is managing the BLE service
     */
    NimBLEService* m_pService = nullptr;

    /**
     * BLE characteristic instance for receiving data
     */
    NimBLECharacteristic* m_pRxCharacteristic = nullptr;

    /**
     * BLE characteristic instance for transmitting data
     */
    NimBLECharacteristic* m_pTxCharacteristic = nullptr;

  public:
    static const char* SERVICE_UUID;
    static const char* RX_UUID;
    static const char* TX_UUID;

    BLESerial() : m_receiveBuffer() {}

    BLESerial(BLESerial const& other) = delete;      // disable copy constructor
    void operator=(BLESerial const& other) = delete; // disable assign constructor

    inline int available() override { return !m_receiveBuffer.empty(); }

    inline int peek() override { return m_receiveBuffer.front(); }

    int read() override
    {
        auto front = m_receiveBuffer.front();
        m_receiveBuffer.pop();
        return front;
    }

    size_t write(const uint8_t* buffer, size_t bufferSize) override
    {
        if (this->m_pTxCharacteristic == nullptr || !this->connected()) {
            return 0;
        }

        this->m_pTxCharacteristic->setValue(const_cast<uint8_t*>(buffer), bufferSize);
        // this->flush();

        return bufferSize;
    }

    size_t write(uint8_t byte) override
    {
        if (this->m_pTxCharacteristic == nullptr || !this->connected()) {
            return 0;
        }

        this->m_pTxCharacteristic->setValue(&byte, 1);
        // this->flush();

        return 1;
    }

    void flush(void) override { this->m_pTxCharacteristic->notify(true); }

    void begin(
      const String& deviceName = String(),
      const char* serviceUuid = SERVICE_UUID,
      const char* rxUuid = RX_UUID,
      const char* txUuid = TX_UUID
    )
    {
        this->begin(deviceName.c_str(), serviceUuid, rxUuid, txUuid);
    }

    /**
     * Begin BLE serial. This will create and start BLE server, service and characteristics.
     *
     * @note This will manage the BLE server, service and characteristics. If you want to manage them yourself, use the
     * other begin().
     *
     * @param deviceName Name of the BLE device
     * @param serviceUuid UUID of the BLE service
     * @param rxUuid UUID of the BLE characteristic for receiving data
     * @param txUuid UUID of the BLE characteristic for transmitting data
     */
    void begin(
      const char* deviceName,
      const char* serviceUuid = SERVICE_UUID,
      const char* rxUuid = RX_UUID,
      const char* txUuid = TX_UUID
    );

    /**
     * Begin BLE serial. This will create and start BLE service and characteristics.
     *
     * @note This will manage the BLE service and characteristics. If you want to manage them yourself, use the other
     * begin().
     *
     * @param pServer BLE server instance
     * @param serviceUuid UUID of the BLE service
     * @param rxUuid UUID of the BLE characteristic for receiving data
     * @param txUuid UUID of the BLE characteristic for transmitting data
     */
    void begin(
      NimBLEServer* pServer,
      const char* serviceUuid = SERVICE_UUID,
      const char* rxUuid = RX_UUID,
      const char* txUuid = TX_UUID
    )
    {
        NimBLEService* pService = pServer->getServiceByUUID(serviceUuid);
        if (pService == nullptr) {
            DEBUG_PRINTF_P(PSTR("Creating BLE service with UUID '%s'"), serviceUuid);
            pService = pServer->createService(serviceUuid);
        } else {
            DEBUG_PRINTF_P(PSTR("BLE service with UUID '%s' already exists"), serviceUuid);
        }

        // Store the service, so we know if we're managing it
        this->m_pService = pService;

        this->begin(pService, rxUuid, txUuid);

        pService->start();
        DEBUG_PRINTF_P(PSTR("Started BLE service\n"));
    }

    /**
     * Begin BLE serial. This will create and start BLE characteristics.
     *
     * @note If you want to create characteristics yourself, use the other begin().
     *
     * @param pService BLE service instance
     * @param rxUuid UUID of the BLE characteristic for receiving data
     * @param txUuid UUID of the BLE characteristic for transmitting data
     */
    void begin(NimBLEService* pService, const char* rxUuid = RX_UUID, const char* txUuid = TX_UUID)
    {
        auto* pRxCharacteristic = pService->getCharacteristic(rxUuid);
        if (pRxCharacteristic == nullptr) {
            DEBUG_PRINTF_P(PSTR("Creating BLE characteristic with UUIDs '%s' (RX)"), rxUuid);
            pRxCharacteristic =
              pService->createCharacteristic(rxUuid,
                                             NIMBLE_PROPERTY::WRITE
                                             | NIMBLE_PROPERTY::WRITE_NR,
                                             2048);
        } else {
            DEBUG_PRINTF_P(PSTR("BLE characteristic with UUID '%s' (RX) already exists"), rxUuid);
        }

        auto* pTxCharacteristic = pService->getCharacteristic(txUuid);
        if (pTxCharacteristic == nullptr) {
            DEBUG_PRINTF_P(PSTR("Creating BLE characteristic with UUIDs '%s' (TX)"), txUuid);
            pTxCharacteristic = pService->createCharacteristic(txUuid,
                                                               NIMBLE_PROPERTY::READ
                                                               | NIMBLE_PROPERTY::NOTIFY,
                                                               2048);
        } else {
            DEBUG_PRINTF_P(PSTR("BLE characteristic with UUID '%s' (TX) already exists"), txUuid);
        }

        this->begin(pRxCharacteristic, pTxCharacteristic);
    }

    /**
     * Begin BLE serial. This will setup the BLE characteristics.
     *
     * @param pServer BLE server instance
     * @param pRxCharacteristic BLE characteristic instance for receiving data
     * @param pTxCharacteristic BLE characteristic instance for transmitting data
     */
    void begin(NimBLECharacteristic* pRxCharacteristic, NimBLECharacteristic* pTxCharacteristic);

    auto connected() -> bool { return m_pServer != nullptr && m_pServer->getConnectedCount() > 0; }

    auto getRxCharacteristic() -> NimBLECharacteristic* { return m_pRxCharacteristic; }

    auto getTxCharacteristic() -> NimBLECharacteristic* { return m_pTxCharacteristic; }
};

template<typename T>
class BLESerialServerCallbacks : public NimBLEServerCallbacks {
  public:
    explicit BLESerialServerCallbacks(BLESerial<T>* bleSerial) : bleSerial(bleSerial) {}

    void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override
    {
        auto* pAdvertising = pServer->getAdvertising();
        if (pAdvertising == nullptr) {
            return;
        }
        pAdvertising->start();
    }

  private:
    BLESerial<T>* bleSerial;
};

template<typename T>
class BLESerialCharacteristicCallbacks : public NimBLECharacteristicCallbacks {
  public:
    explicit BLESerialCharacteristicCallbacks(BLESerial<T>* bleSerial) : bleSerial(bleSerial) {}

    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override
    {
        if (pCharacteristic != bleSerial->m_pRxCharacteristic) {
            return;
        }

        auto rxValue = pCharacteristic->getValue();
        for (int i = 0; i < rxValue.length(); i++) {
            bleSerial->m_receiveBuffer.push(rxValue[i]);
        }
    }

  private:
    BLESerial<T>* bleSerial;
};

template<typename T>
const char* BLESerial<T>::SERVICE_UUID = "6E400001-B5A3-F393-E0A9-E50E24DCCA9E";

template<typename T>
const char* BLESerial<T>::RX_UUID = "6E400002-B5A3-F393-E0A9-E50E24DCCA9E";

template<typename T>
const char* BLESerial<T>::TX_UUID = "6E400003-B5A3-F393-E0A9-E50E24DCCA9E";

template<typename T>
void BLESerial<T>::begin(const char* deviceName, const char* serviceUuid, const char* rxUuid, const char* txUuid)
{
    // Create the BLE Device
    DEBUG_PRINTF_P(PSTR("Initializing BLE device with name '%s'"), deviceName);
    NimBLEDevice::init(deviceName);
    NimBLEDevice::setMTU(512);

    log_d("Creating BLE server");
    NimBLEServer* pServer = NimBLEDevice::createServer();
    pServer->setCallbacks(new BLESerialServerCallbacks<T>(this));

    // Store the server, so we know if we're managing it
    this->m_pServer = pServer;

    this->begin(pServer, serviceUuid, rxUuid, txUuid);

    NimBLEAdvertising* pAdvertising = pServer->getAdvertising();
    pAdvertising->setName(deviceName);
    pAdvertising->start();
    DEBUG_PRINTF_P(PSTR("Started BLE advertising\n"));
}

template<typename T>
void BLESerial<T>::begin(NimBLECharacteristic* pRxCharacteristic, NimBLECharacteristic* pTxCharacteristic)
{
    // Store the characteristics, so we know if we're managing them
    this->m_pRxCharacteristic = pRxCharacteristic;
    this->m_pTxCharacteristic = pTxCharacteristic;

    this->m_pRxCharacteristic->setCallbacks(new BLESerialCharacteristicCallbacks<T>(this));
}

#endif // BLESERIAL_H