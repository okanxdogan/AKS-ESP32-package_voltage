#include "CanSniffer.h"

const BaudConfig CanSniffer::BAUD_LIST[] = {
    {CAN_500KBPS, "500 kbps"},
    {CAN_250KBPS, "250 kbps"},
    {CAN_125KBPS, "125 kbps"},
};

CanSniffer::CanSniffer()
    : mcp(CAN_CS_PIN),
      lastPackVoltageDeciV(0),
      lastPrintTime(0) {}

void CanSniffer::begin() {
    Serial.println("\n========================================");
    Serial.println("  c-BMS24 Pack Voltage Monitor");
    Serial.println("  Lithium Balance — LiBat CANbus v1.0");
    Serial.println("========================================");

    setupInterface();

    if (!initMCP()) {
        Serial.println("\n[HATA] MCP2515 ile SPI iletisimi kurulamadi!");
        Serial.println("Kontrol listesi:");
        Serial.println("  1. VCC -> 5V (3.3V yetmeyebilir)");
        Serial.println("  2. GND -> GND (ortak)");
        Serial.println("  3. CS  -> GPIO5");
        Serial.println("  4. SO  -> GPIO19 (MISO)");
        Serial.println("  5. SI  -> GPIO23 (MOSI)");
        Serial.println("  6. SCK -> GPIO18");
        Serial.println("  7. INT -> GPIO4");
        while (1) {
            delay(1000);
        }
    }

    mcp.setMode(MCP_NORMAL);
    pinMode(CAN_INT_PIN, INPUT);

    Serial.println("\n[BILGI] CAN bus dinleniyor...");
    Serial.println("[BILGI] Hedef CAN ID: 0xE000 (c-BMS24 Status)");
    Serial.println("[BILGI] Pack Voltage: Byte[2:3], Big-Endian, 0.1V/bit");
    Serial.println("--------------------------------------------------\n");
}

void CanSniffer::handle() {
    if (digitalRead(CAN_INT_PIN) != LOW) {
        return;
    }

    unsigned long rxId = 0;
    unsigned char dlc = 0;
    unsigned char buf[8] = {0};

    mcp.readMsgBuf(&rxId, &dlc, buf);

    // Extended frame flag'ini temizle, gerçek 29-bit ID'yi al
    rxId &= 0x1FFFFFFF;

    // Sadece 0xE000 (Status + Pack Voltage) mesajını parse et
    if (rxId == CBMS_ID_STATUS) {
        parsePackVoltage(buf, dlc);
    }
}

void CanSniffer::parsePackVoltage(const unsigned char* buf, uint8_t dlc) {
    if (dlc < 4) {
        return;  // En az 4 byte gerekli (byte 2-3 pack voltage)
    }

    // Byte[2] = High byte, Byte[3] = Low byte (Big-Endian)
    // Değer deci-volt cinsinden: ör. 0x0310 = 784 → 78.4V
    uint16_t packVDeciV = ((uint16_t)buf[2] << 8) | buf[3];

    lastPackVoltageDeciV = packVDeciV;

    // Terminalde her 500ms'de bir yazdır (flood'u önlemek için)
    unsigned long now = millis();
    if (now - lastPrintTime >= 500) {
        lastPrintTime = now;

        float packVoltage = packVDeciV / 10.0f;

        Serial.printf("Paket Voltaji: %.1f V\n", packVoltage);
    }
}

void CanSniffer::setupInterface() {
    SPI.begin(18, 19, 23, CAN_CS_PIN);
}

bool CanSniffer::initMCP() {
    for (const auto& cfg : BAUD_LIST) {
        Serial.printf("Deneniyor: %s @ 8MHz ... ", cfg.label);
        if (mcp.begin(MCP_ANY, cfg.baud, MCP_8MHZ) == CAN_OK) {
            Serial.printf("BASARILI!\n");
            return true;
        }
        Serial.println("basarisiz.");
        delay(100);
    }
    return false;
}
