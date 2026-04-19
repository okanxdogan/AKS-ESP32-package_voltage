#include "CanSniffer.h"

const BaudConfig CanSniffer::BAUD_LIST[] = {
    {CAN_500KBPS, "500 kbps"},
    {CAN_250KBPS, "250 kbps"},
    {CAN_125KBPS, "125 kbps"},
};

CanSniffer::CanSniffer()
    : mcp(CAN_CS_PIN),
      seenIds{0},
      seenCounts{0},
      seenCount(0),
      lastSummary(0) {}

void CanSniffer::begin() {
    Serial.println("\n========================================");
    Serial.println("   BMS CAN Sniffer — MCP2515 + ESP32");
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

    Serial.println("\n--- Dinleniyor ---");
    Serial.println(
        "TYPE     | ID           | DLC | BYTES (HEX)                     | "
        "BYTES (DEC)");
    Serial.println(
        "---------|--------------|-----|----------------------------------|----"
        "------------");
}

void CanSniffer::handle() {
    printSummary();

    if (digitalRead(CAN_INT_PIN) != LOW) {
        return;
    }

    unsigned long rxId = 0;
    unsigned char dlc = 0;
    unsigned char buf[8] = {0};

    mcp.readMsgBuf(&rxId, &dlc, buf);

    bool isExtended = (rxId & 0x80000000) != 0;
    rxId &= 0x1FFFFFFF;

    recordId(rxId);

    if (isExtended) {
        Serial.printf("EXT      | 0x%08lX   |  %d  |", rxId, dlc);
    } else {
        Serial.printf("STD      | 0x%03lX          |  %d  |", rxId, dlc);
    }

    for (uint8_t i = 0; i < dlc; i++) {
        Serial.printf(" %02X", buf[i]);
    }
    for (uint8_t i = dlc; i < 8; i++) {
        Serial.print("   ");
    }

    Serial.print("  |");
    for (uint8_t i = 0; i < dlc; i++) {
        Serial.printf(" %3d", buf[i]);
    }
    Serial.println();
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

void CanSniffer::recordId(uint32_t id) {
    for (uint8_t i = 0; i < seenCount; i++) {
        if (seenIds[i] == id) {
            seenCounts[i]++;
            return;
        }
    }
    if (seenCount < 64) {
        seenIds[seenCount] = id;
        seenCounts[seenCount] = 1;
        seenCount++;
    }
}

void CanSniffer::printSummary() {
    if (millis() - lastSummary < 5000) {
        return;
    }
    lastSummary = millis();

    if (seenCount == 0) {
        Serial.println(
            "\n[OZET] Hic frame alınamadı — BMS baglantisini kontrol et veya "
            "baud rate yanlis.");
        return;
    }

    Serial.printf("\n[OZET] Toplam %d farklı ID goruldu:\n", seenCount);
    for (uint8_t i = 0; i < seenCount; i++) {
        Serial.printf("  0x%08lX  →  %lu frame\n", seenIds[i], seenCounts[i]);
    }
    Serial.println();
}
