#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <mcp_can.h>

#define CAN_CS_PIN 5
#define CAN_INT_PIN 4

// ── Lithium Balance c-BMS24 — LiBat CANbus v1.0 ──
// Base ID = 0xE000 (extended CAN, 29-bit)
// Pack Voltage → CAN ID 0xE000, Byte[2:3], Big-Endian, uint16, 0.1V/bit
#define CBMS_BASE_ID      0x0000E000UL
#define CBMS_ID_STATUS    (CBMS_BASE_ID + 0x00)   // Status + Pack Voltage

struct BaudConfig {
    uint8_t baud;
    const char* label;
};

class CanSniffer {
   public:
    CanSniffer();
    void begin();
    void handle();

   private:
    void setupInterface();
    bool initMCP();
    void parsePackVoltage(const unsigned char* buf, uint8_t dlc);

    MCP_CAN mcp;
    static const BaudConfig BAUD_LIST[];

    // Son okunan paket voltajı (deci-volt cinsinden, ör. 784 = 78.4V)
    uint16_t lastPackVoltageDeciV;
    unsigned long lastPrintTime;
};
